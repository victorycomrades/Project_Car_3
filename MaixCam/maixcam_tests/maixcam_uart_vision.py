"""
MaixCam UART 双向通信视觉服务 — 主生产脚本
=============================================

功能：
- 通过 UART 串口与 Jetson Nano 双向通信
- 接收 Jetson 下发的 MODE 指令切换视觉模式
- 发送结构化视觉识别结果（QR/BLOB/RING/LINE）
- 所有坐标输出 (dx, dy) 为相对于相机画面中心的偏差，供机械臂使用
- 在 MaixCam 屏幕上显示调试画面

硬件接线（MaixCam UART1）：
  MaixCam A19 (TX)  →  Jetson RX
  MaixCam A18 (RX)  →  Jetson TX
  MaixCam GND       →  Jetson GND
  IO 电压 3.3V，如需接 5V 设备请加电平转换

协议格式（MaixCam → Jetson）：
  QR,payload
  BLOB,color,dx,dy,cx,cy,w,h,area
  RING,color,dx,dy,cx,cy,radius,score,density,ratio,source
  LINE,dx,theta
  NONE,type[,color]
  INFO,text

协议格式（Jetson → MaixCam）：
  MODE,QR
  MODE,BLOB,RED|GREEN|BLUE|ALL
  MODE,RING,RED|GREEN|BLUE|ALL
  MODE,LINE
  MODE,IDLE
  PING
"""

from maix import app, camera, display, image, pinmap, time, uart

# ---- 基本配置 ---------------------------------------------------------------
WIDTH = 320
HEIGHT = 240

# UART 配置
# USB 转 TTL 接线时使用 UART1=/dev/ttyS1 (A19 TX, A18 RX)。
# /dev/ttyGS0 是 MaixCam 自身 USB 虚拟串口，只有直接用 MaixCam USB 线通信时才放前面。
UART_DEVICES = ("/dev/ttyS1", "/dev/ttyS0", "/dev/ttyGS0")
UART_BAUD = 115200

# 默认视觉模式（Jetson 可通过 MODE 命令覆盖）
MODE = "QR"

# ---- 颜色阈值配置（使用 MaixCam Find Blobs 工具在实际场地标定后回填）--------

COLOR_CONFIGS = {
    "RED": ([0, 90, 35, 90, -10, 90], image.COLOR_RED),
    "GREEN": ([0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    "BLUE": ([0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
}

# 色环 LAB 阈值（可与物料阈值不同，虚线圆通常需要更宽容的阈值）
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

PIXELS_THRESHOLD = 200
AREA_THRESHOLD = 200

# 巡线阈值
LINE_THRESHOLD = [20, 90, -15, 15, -15, 15]
LINE_ROI = [0, HEIGHT // 2, WIDTH, HEIGHT // 2]

# 色环检测参数
RING_PIXELS_THRESHOLD = 50
RING_AREA_THRESHOLD = 50
RING_MERGE_MARGIN = 22
RING_MIN_SIZE = 22
RING_MAX_SIZE = 210
RING_MIN_SQUARE_RATIO = 68
RING_MAX_INK_DENSITY = 55

# 多圆霍夫检测开关及参数
USE_MULTI_CIRCLE = True
CIRCLE_THRESHOLD = 1900
CIRCLE_STRIDE = 3
CIRCLE_R_STEP = 2
CIRCLE_MARGIN_XY = 7
CIRCLE_MARGIN_R = 4
CENTER_GATE_PIXELS = 22
RING_ROI = [0, 0, WIDTH, HEIGHT]

# 平滑系数：固定安装后建议 0.35
SMOOTH_ALPHA = 1.0

# ---- UART 初始化 -------------------------------------------------------------

uart_dev = None
rx_buffer = ""


def uart_init():
    """初始化 UART 串口"""
    global uart_dev
    last_error = None
    for device in UART_DEVICES:
        try:
            if device == "/dev/ttyS1":
                pinmap.set_pin_function("A18", "UART1_RX")
                pinmap.set_pin_function("A19", "UART1_TX")
                print("UART1 pins configured: A18=RX, A19=TX")
            uart_dev = uart.UART(device, UART_BAUD)
            print("UART opened: %s @ %d" % (device, UART_BAUD))
            return
        except Exception as err:
            last_error = err
            print("UART open failed: %s %s" % (device, err))
    raise RuntimeError("No UART device available: %s" % last_error)


def uart_write(text):
    """通过 UART 发送一行文本"""
    if uart_dev is None:
        return
    try:
        uart_dev.write_str(str(text) + "\n")
    except Exception:
        pass


def uart_read_cmds():
    """非阻塞读取 UART 数据，处理完整行命令"""
    global rx_buffer, MODE
    if uart_dev is None:
        return
    try:
        data = uart_dev.read()
        if data is None:
            return
    except Exception:
        return

    if isinstance(data, str):
        text = data
    else:
        try:
            text = data.decode("utf-8", errors="replace")
        except TypeError:
            text = data.decode("utf-8")

    rx_buffer += text
    while "\n" in rx_buffer:
        line, rx_buffer = rx_buffer.split("\n", 1)
        line = line.strip()
        if line:
            handle_command(line)


def handle_command(line):
    """解析并执行 Jetson 发来的命令"""
    global MODE
    parts = [p.strip().upper() for p in line.split(",")]
    if not parts:
        return

    if parts[0] == "MODE":
        MODE = normalize_mode(parts)
        uart_write("INFO,MODE,%s" % MODE)
    elif parts[0] == "PING":
        uart_write("PONG,MAIXCAM")
    else:
        uart_write("ERR,UNKNOWN_COMMAND,%s" % line)


def normalize_mode(parts):
    """将 ['MODE', 'BLOB', 'RED'] 转为 'BLOB_RED' 等内部模式名"""
    if len(parts) < 2:
        return "IDLE"
    if parts[1] == "BLOB":
        color = parts[2] if len(parts) >= 3 else "ALL"
        return "BLOB_" + color
    if parts[1] == "RING":
        color = parts[2] if len(parts) >= 3 else "ALL"
        return "RING_" + color
    return parts[1]


# ---- 视觉辅助函数 ------------------------------------------------------------


def find_largest_blob(img, threshold, pixels_th=PIXELS_THRESHOLD, area_th=AREA_THRESHOLD):
    blobs = img.find_blobs(
        [threshold],
        pixels_threshold=pixels_th,
        area_threshold=area_th,
        merge=True,
    )
    if not blobs:
        return None
    return max(blobs, key=lambda b: b[2] * b[3])


def blob_report(color_name, blob):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
    area = w * h
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    return "BLOB,%s,%d,%d,%d,%d,%d,%d,%d" % (color_name, dx, dy, cx, cy, w, h, area)


def draw_blob(img, color_name, blob, draw_color):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    img.draw_rect(x, y, w, h, draw_color, 2)
    img.draw_cross(cx, cy, draw_color, 10, 2)
    img.draw_string(x, max(0, y - 16),
                    "%s d(%d,%d)" % (color_name, dx, dy), draw_color)


# ---- 色环检测函数 ------------------------------------------------------------
_smooth_centers = {}


def ring_density(blob):
    area = blob.w() * blob.h()
    if area <= 0:
        return 100
    return blob.pixels() * 100 // area


def ring_square_ratio(blob):
    w, h = blob.w(), blob.h()
    if w <= 0 or h <= 0:
        return 0
    return min(w, h) * 100 // max(w, h)


def is_ring_like(blob):
    w, h = blob.w(), blob.h()
    if w < RING_MIN_SIZE or h < RING_MIN_SIZE:
        return False
    if w > RING_MAX_SIZE or h > RING_MAX_SIZE:
        return False
    if ring_square_ratio(blob) < RING_MIN_SQUARE_RATIO:
        return False
    if ring_density(blob) > RING_MAX_INK_DENSITY:
        return False
    return True


def ring_score(blob):
    size_score = blob.w() * blob.h()
    square_score = ring_square_ratio(blob) * 30
    density_score = max(0, 60 - ring_density(blob)) * 10
    return size_score + square_score + density_score


def find_ring_blob(img, threshold):
    blobs = img.find_blobs(
        [threshold],
        roi=RING_ROI,
        pixels_threshold=RING_PIXELS_THRESHOLD,
        area_threshold=RING_AREA_THRESHOLD,
        merge=True,
        margin=RING_MERGE_MARGIN,
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


def blob_fallback(blob):
    cx = blob.x() + blob.w() // 2
    cy = blob.y() + blob.h() // 2
    radius = max(blob.w(), blob.h()) // 2
    return cx, cy, radius, "BLOB"


def refine_multi_circle(img, blob, fallback):
    """在 blob 小 ROI 内检测多个真实圆并加权融合圆心"""
    if not USE_MULTI_CIRCLE:
        return fallback

    pad = 10
    bx, by, br, _ = fallback
    roi = clamp_roi(blob.x() - pad, blob.y() - pad,
                    blob.w() + pad * 2, blob.h() + pad * 2)
    max_side = max(roi[2], roi[3])
    min_side = min(roi[2], roi[3])
    r_min = max(6, min_side // 5)
    r_max = max(r_min + 2, max_side // 2 + 4)

    try:
        circles = img.find_circles(
            roi=roi,
            x_stride=CIRCLE_STRIDE,
            y_stride=CIRCLE_STRIDE,
            threshold=CIRCLE_THRESHOLD,
            x_margin=CIRCLE_MARGIN_XY,
            y_margin=CIRCLE_MARGIN_XY,
            r_margin=CIRCLE_MARGIN_R,
            r_min=r_min,
            r_max=r_max,
            r_step=CIRCLE_R_STEP,
        )
    except Exception:
        return fallback

    if not circles:
        return fallback

    # 门控：只保留圆心靠近粗定位中心的圆
    selected = []
    for c in circles:
        d2 = (c.x() - bx) ** 2 + (c.y() - by) ** 2
        if d2 <= CENTER_GATE_PIXELS ** 2:
            selected.append(c)
    if not selected:
        selected = sorted(circles, key=lambda c: c.magnitude(), reverse=True)[:3]

    weight_sum = 0
    sx = sy = sr = 0
    for c in selected:
        w = c.magnitude()
        if w <= 0:
            w = 1
        sx += c.x() * w
        sy += c.y() * w
        sr += c.r() * w
        weight_sum += w

    cx = sx // weight_sum
    cy = sy // weight_sum
    radius = sr // weight_sum
    source = "MULTI_HOUGH%d" % len(selected) if len(selected) >= 2 else "SINGLE_HOUGH%d" % len(selected)
    return cx, cy, radius, source


def smooth_center(color_name, cx, cy):
    if color_name not in _smooth_centers:
        _smooth_centers[color_name] = [cx, cy]
        return cx, cy
    old_x, old_y = _smooth_centers[color_name]
    new_x = int(old_x * (1.0 - SMOOTH_ALPHA) + cx * SMOOTH_ALPHA)
    new_y = int(old_y * (1.0 - SMOOTH_ALPHA) + cy * SMOOTH_ALPHA)
    _smooth_centers[color_name] = [new_x, new_y]
    return new_x, new_y


def detect_color_ring(img, color_name):
    config = RING_COLORS[color_name]
    blob = find_ring_blob(img, config["threshold"])
    if blob is None:
        return None

    fallback = blob_fallback(blob)
    cx, cy, radius, source = refine_multi_circle(img, blob, fallback)
    cx, cy = smooth_center(color_name, cx, cy)

    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    density = ring_density(blob)
    ratio = ring_square_ratio(blob)
    score = ring_score(blob)

    report = "RING,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s" % (
        color_name, dx, dy, cx, cy, radius, score, density, ratio, source)

    return {
        "color_name": color_name,
        "cx": cx, "cy": cy, "radius": radius,
        "draw_color": config["draw"],
        "report": report,
    }


def draw_ring_result(img, result):
    color_name = result["color_name"]
    cx, cy, radius = result["cx"], result["cy"], result["radius"]
    draw_color = result["draw_color"]
    if radius > 3:
        img.draw_circle(cx, cy, radius, draw_color, 2)
    img.draw_string(max(0, cx - 18), max(0, cy - radius - 16), color_name, draw_color)
    img.draw_cross(cx, cy, image.COLOR_BLUE, 12, 3)
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    img.draw_string(max(0, cx + 8), max(0, cy - 8), "d(%d,%d)" % (dx, dy), image.COLOR_BLUE)


# ---- 各视觉模式 --------------------------------------------------------------


def run_qr(img, frame_id):
    qrcodes = img.find_qrcodes()
    if not qrcodes:
        img.draw_string(0, 0, "QR: none", image.COLOR_RED)
        if frame_id % 20 == 0:
            uart_write("NONE,QR")
        return

    qr = qrcodes[0]
    payload = qr.payload()
    corners = qr.corners()
    for i in range(4):
        x1, y1 = corners[i][0], corners[i][1]
        x2, y2 = corners[(i + 1) % 4][0], corners[(i + 1) % 4][1]
        img.draw_line(x1, y1, x2, y2, image.COLOR_RED, 2)

    img.draw_string(qr.x(), max(0, qr.y() - 16), payload, image.COLOR_RED)

    if frame_id % 5 == 0:
        uart_write("QR,%s" % payload)


def run_blob(img, target, frame_id):
    colors = ("RED", "GREEN", "BLUE") if target == "ALL" else (target,)
    found = False
    for color_name in colors:
        if color_name not in COLOR_CONFIGS:
            continue
        threshold, draw_color = COLOR_CONFIGS[color_name]
        blob = find_largest_blob(img, threshold)
        if blob is None:
            continue
        found = True
        draw_blob(img, color_name, blob, draw_color)
        if frame_id % 5 == 0:
            uart_write(blob_report(color_name, blob))
    if not found:
        img.draw_string(0, 0, "BLOB %s: none" % target, image.COLOR_RED)
        if frame_id % 20 == 0:
            uart_write("NONE,BLOB,%s" % target)


def run_ring(img, target, frame_id):
    colors = ("RED", "GREEN", "BLUE") if target == "ALL" else (target,)
    results = []
    for color_name in colors:
        if color_name not in RING_COLORS:
            continue
        result = detect_color_ring(img, color_name)
        if result:
            results.append(result)

    for r in results:
        draw_ring_result(img, r)

    if frame_id % 5 == 0:
        if results:
            for r in results:
                uart_write(r["report"])
        else:
            uart_write("NONE,RING,%s" % target)


def run_line(img, frame_id):
    img.draw_rect(LINE_ROI[0], LINE_ROI[1], LINE_ROI[2], LINE_ROI[3],
                  image.COLOR_BLUE, 1)
    lines = img.get_regression([LINE_THRESHOLD], roi=LINE_ROI, area_threshold=100)
    if not lines:
        img.draw_string(0, 0, "LINE: none", image.COLOR_RED)
        if frame_id % 20 == 0:
            uart_write("NONE,LINE")
        return

    line = lines[0]
    img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.COLOR_GREEN, 2)

    theta = line.theta()
    rho = line.rho()
    if theta > 90:
        theta_out = 270 - theta
    else:
        theta_out = 90 - theta
    dx = rho - WIDTH // 2

    img.draw_string(0, 0, "LINE dx=%d theta=%d" % (dx, theta_out), image.COLOR_GREEN)
    if frame_id % 5 == 0:
        uart_write("LINE,%d,%d" % (dx, theta_out))


# ---- 主循环 ------------------------------------------------------------------

cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

uart_init()
print("MaixCam UART vision server started.")
print("UART baud: %d" % UART_BAUD)
print("Default MODE: %s" % MODE)
uart_write("HELLO,MAIXCAM_UART,%d" % UART_BAUD)

while not app.need_exit():
    uart_read_cmds()

    img = cam.read()
    img.draw_cross(WIDTH // 2, HEIGHT // 2, image.COLOR_WHITE, 6, 1)
    img.draw_string(0, HEIGHT - 16, "MODE:%s" % MODE, image.COLOR_WHITE)

    if MODE == "QR":
        run_qr(img, frame_id)
    elif MODE.startswith("BLOB_"):
        run_blob(img, MODE.split("_", 1)[1], frame_id)
    elif MODE.startswith("RING_"):
        run_ring(img, MODE.split("_", 1)[1], frame_id)
    elif MODE == "LINE":
        run_line(img, frame_id)
    elif MODE == "IDLE":
        if frame_id % 60 == 0:
            uart_write("INFO,IDLE")
    else:
        img.draw_string(0, 0, "Unknown MODE: " + MODE, image.COLOR_RED)
        if frame_id % 20 == 0:
            uart_write("ERR,UNKNOWN_MODE,%s" % MODE)

    frame_id += 1
    disp.show(img)
