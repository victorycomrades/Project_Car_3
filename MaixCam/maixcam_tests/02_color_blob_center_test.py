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

COLOR_CONFIGS = [
    ("RED", [0, 90, 35, 90, -10, 90], image.COLOR_RED),
    ("GREEN", [0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    ("BLUE", [0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
]

PIXELS_THRESHOLD = 200
AREA_THRESHOLD = 200


def find_largest_blob(img, threshold):
    blobs = img.find_blobs(
        [threshold],
        pixels_threshold=PIXELS_THRESHOLD,
        area_threshold=AREA_THRESHOLD,
        merge=True,
    )
    if not blobs:
        return None
    return max(blobs, key=lambda b: b[2] * b[3])


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("Color blob center test started (dx,dy = offset from image center).")

while not app.need_exit():
    img = cam.read()
    img_cx = WIDTH // 2
    img_cy = HEIGHT // 2
    found = []

    img.draw_cross(img_cx, img_cy, image.COLOR_WHITE, 6, 1)

    for name, threshold, draw_color in COLOR_CONFIGS:
        blob = find_largest_blob(img, threshold)
        if blob is None:
            continue

        x, y, w, h = blob[0], blob[1], blob[2], blob[3]
        cx = x + w // 2
        cy = y + h // 2
        area = w * h
        dx = cx - img_cx
        dy = cy - img_cy

        img.draw_rect(x, y, w, h, draw_color, 2)
        img.draw_cross(cx, cy, draw_color, 10, 2)
        img.draw_string(x, max(0, y - 16),
                        "%s d(%d,%d)" % (name, dx, dy), draw_color)

        found.append("%s,%d,%d,%d,%d,%d,%d,%d" % (name, dx, dy, cx, cy, w, h, area))

    if frame_id % 10 == 0:
        if found:
            for item in found:
                uart_write("BLOB," + item)
        else:
            uart_write("NONE,BLOB,ALL")

    frame_id += 1
    disp.show(img)
