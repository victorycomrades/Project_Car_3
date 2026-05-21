from maix import app, camera, display, image

WIDTH = 320
HEIGHT = 240

# ---- UART 配置（设为 None 则仅打印到终端）-----------------------------------
UART_DEVICE = None      # 改为 "/dev/ttyS0" 或 "/dev/ttyS1" 启用串口输出
UART_BAUD = 115200

uart_dev = None

def uart_init():
    global uart_dev
    if UART_DEVICE is None:
        return
    try:
        if UART_DEVICE == "/dev/ttyS1":
            from maix import pinmap
            pinmap.set_pin_function("A18", "UART1_RX")
            pinmap.set_pin_function("A19", "UART1_TX")
        from maix import uart
        uart_dev = uart.UART(UART_DEVICE, UART_BAUD)
        print("UART opened:", UART_DEVICE)
    except Exception as err:
        print("UART open failed:", err)

def uart_write(text):
    print(text)
    if uart_dev is None:
        return
    try:
        uart_dev.write_str(str(text) + "\n")
    except Exception:
        pass

uart_init()

TARGET = "ALL"
ROI = [0, 0, WIDTH, HEIGHT]

RING_COLORS = {
    "RED": {
        "threshold": [0, 95, 25, 90, -10, 90],
        "draw": image.COLOR_RED,
    },
    "GREEN": {
        "threshold": [0, 95, -120, -10, -20, 90],
        "draw": image.COLOR_GREEN,
    },
    "BLUE": {
        "threshold": [0, 95, -20, 80, -128, -20],
        "draw": image.COLOR_BLUE,
    },
}

PIXELS_THRESHOLD = 50
AREA_THRESHOLD = 50
MERGE_MARGIN = 22

MIN_RING_SIZE = 22
MAX_RING_SIZE = 210
MIN_SQUARE_RATIO = 68
MAX_INK_DENSITY = 55

USE_HOUGH_REFINE = False
CIRCLE_THRESHOLD = 2800
R_MIN = 10
R_MAX = 95
R_STEP = 3

DRAW_CONCENTRIC = True
RING_RATIO_LIST = [1.00, 0.84, 0.68, 0.52]

SMOOTH_ALPHA = 0.35
smooth_centers = {}


def wanted_colors():
    if TARGET == "ALL":
        return ["RED", "GREEN", "BLUE"]
    if TARGET in RING_COLORS:
        return [TARGET]
    raise ValueError("TARGET must be ALL, RED, GREEN, or BLUE")


def density_percent(blob):
    area = blob.w() * blob.h()
    if area <= 0:
        return 100
    return blob.pixels() * 100 // area


def square_ratio(blob):
    w, h = blob.w(), blob.h()
    if w <= 0 or h <= 0:
        return 0
    return min(w, h) * 100 // max(w, h)


def is_ring_like(blob):
    w, h = blob.w(), blob.h()
    if w < MIN_RING_SIZE or h < MIN_RING_SIZE:
        return False
    if w > MAX_RING_SIZE or h > MAX_RING_SIZE:
        return False
    if square_ratio(blob) < MIN_SQUARE_RATIO:
        return False
    if density_percent(blob) > MAX_INK_DENSITY:
        return False
    return True


def ring_score(blob):
    size_score = blob.w() * blob.h()
    square_score = square_ratio(blob) * 30
    density_score = max(0, 60 - density_percent(blob)) * 10
    return size_score + square_score + density_score


def find_best_ring_blob(img, threshold):
    blobs = img.find_blobs(
        [threshold],
        roi=ROI,
        pixels_threshold=PIXELS_THRESHOLD,
        area_threshold=AREA_THRESHOLD,
        merge=True,
        margin=MERGE_MARGIN,
    )
    candidates = [b for b in blobs if is_ring_like(b)]
    if not candidates:
        return None
    return max(candidates, key=ring_score)


def clamp_roi(x, y, w, h):
    if x < 0:
        w += x; x = 0
    if y < 0:
        h += y; y = 0
    if x + w > WIDTH:
        w = WIDTH - x
    if y + h > HEIGHT:
        h = HEIGHT - y
    return [x, y, max(1, w), max(1, h)]


def estimate_from_blob(blob):
    cx = blob.x() + blob.w() // 2
    cy = blob.y() + blob.h() // 2
    radius = (blob.w() + blob.h()) // 4
    score = ring_score(blob)
    return cx, cy, radius, score, "BLOB"


def refine_with_hough(img, blob, fallback, draw_color):
    if not USE_HOUGH_REFINE:
        return fallback

    x, y, w, h = blob.x(), blob.y(), blob.w(), blob.h()
    circle_roi = clamp_roi(x - 8, y - 8, w + 16, h + 16)

    try:
        circles = img.find_circles(
            roi=circle_roi,
            x_stride=4,
            y_stride=4,
            threshold=CIRCLE_THRESHOLD,
            x_margin=8,
            y_margin=8,
            r_margin=8,
            r_min=R_MIN,
            r_max=R_MAX,
            r_step=R_STEP,
        )
    except Exception as err:
        print("WARN,HOUGH_FAIL,%s" % err)
        return fallback

    if not circles:
        return fallback

    fx, fy, _, _, _ = fallback
    selected = []
    for c in circles:
        dx = c.x() - fx
        dy = c.y() - fy
        if dx * dx + dy * dy <= 25 * 25:
            selected.append(c)
    if not selected:
        selected = circles[:3]

    weight_sum = 0
    sx = sy = sr = 0
    score = 0
    for c in selected:
        weight = c.magnitude()
        if weight <= 0:
            weight = 1
        sx += c.x() * weight
        sy += c.y() * weight
        sr += c.r() * weight
        score += c.magnitude()
        weight_sum += weight

    cx = sx // weight_sum
    cy = sy // weight_sum
    radius = sr // weight_sum
    return cx, cy, radius, score, "HOUGH%d" % len(selected)


def smooth(color_name, cx, cy):
    if color_name not in smooth_centers:
        smooth_centers[color_name] = [cx, cy]
        return cx, cy
    old_x, old_y = smooth_centers[color_name]
    new_x = int(old_x * (1.0 - SMOOTH_ALPHA) + cx * SMOOTH_ALPHA)
    new_y = int(old_y * (1.0 - SMOOTH_ALPHA) + cy * SMOOTH_ALPHA)
    smooth_centers[color_name] = [new_x, new_y]
    return new_x, new_y


def draw_ring_overlay(img, color_name, cx, cy, radius, draw_color):
    if radius > 3:
        if DRAW_CONCENTRIC:
            for ratio in RING_RATIO_LIST:
                r = int(radius * ratio)
                if r > 3:
                    img.draw_circle(cx, cy, r, draw_color, 2)
        else:
            img.draw_circle(cx, cy, radius, draw_color, 2)

    img.draw_string(max(0, cx - 18), max(0, cy - radius - 16),
                    color_name, draw_color)
    img.draw_cross(cx, cy, image.COLOR_BLUE, 12, 3)
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    img.draw_string(max(0, cx + 8), max(0, cy - 8),
                    "d(%d,%d)" % (dx, dy), image.COLOR_BLUE)


def detect_color_ring(img, color_name):
    config = RING_COLORS[color_name]
    draw_color = config["draw"]
    blob = find_best_ring_blob(img, config["threshold"])
    if blob is None:
        return None

    result = estimate_from_blob(blob)
    result = refine_with_hough(img, blob, result, draw_color)
    cx, cy, radius, score, source = result
    cx, cy = smooth(color_name, cx, cy)

    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    density = density_percent(blob)
    ratio = square_ratio(blob)

    report = "RING,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s" % (
        color_name, dx, dy, cx, cy, radius, score, density, ratio, source)

    return {
        "color_name": color_name,
        "cx": cx, "cy": cy, "radius": radius,
        "draw_color": draw_color,
        "report": report,
    }


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("RGB color ring test started (dx,dy = offset from image center).")
print("TARGET =", TARGET)

while not app.need_exit():
    img = cam.read()
    img.draw_cross(WIDTH // 2, HEIGHT // 2, image.COLOR_WHITE, 6, 1)

    results = []
    for color_name in wanted_colors():
        result = detect_color_ring(img, color_name)
        if result:
            results.append(result)

    for result in results:
        draw_ring_overlay(img, result["color_name"], result["cx"],
                          result["cy"], result["radius"], result["draw_color"])

    if frame_id % 8 == 0:
        if results:
            for result in results:
                uart_write(result["report"])
        else:
            uart_write("NONE,RING,%s" % TARGET)

    frame_id += 1
    disp.show(img)
