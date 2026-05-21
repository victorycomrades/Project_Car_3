from maix import app, camera, display, image, time

WIDTH = 320
HEIGHT = 240

# ---- UART 配置（设为 None 则仅打印到终端）-----------------------------------
UART_DEVICE = None      # 改为 "/dev/ttyS0" 或 "/dev/ttyS1" 启用串口通信
UART_BAUD = 115200

uart_dev = None
rx_buffer = ""

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
        print("UART opened:", UART_DEVICE, "@", UART_BAUD)
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

def uart_read_cmds():
    global rx_buffer
    global MODE
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
        line = line.strip().upper()
        if not line:
            continue
        parts = [p.strip() for p in line.split(",")]
        if parts and parts[0] == "MODE":
            new_mode = parts[1] if len(parts) > 1 else "QR"
            if len(parts) >= 3:
                new_mode = "BLOB_" + parts[2] if parts[1] == "BLOB" else \
                           "RING_" + parts[2] if parts[1] == "RING" else new_mode
            if new_mode in ("QR", "BLOB_RED", "BLOB_GREEN", "BLOB_BLUE",
                            "BLOB_ALL", "RING_RED", "RING_GREEN", "RING_BLUE",
                            "RING_ALL", "LINE", "IDLE"):
                MODE = new_mode
                uart_write("INFO,MODE,%s" % MODE)

uart_init()

# 默认视觉模式（UART 未连接时手动修改此处）
MODE = "QR"

COLOR_CONFIGS = {
    "RED": ([0, 90, 35, 90, -10, 90], image.COLOR_RED),
    "GREEN": ([0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    "BLUE": ([0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
}

PIXELS_THRESHOLD = 200
AREA_THRESHOLD = 200

LINE_THRESHOLD = [20, 90, -15, 15, -15, 15]
LINE_ROI = [0, HEIGHT // 2, WIDTH, HEIGHT // 2]


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


def draw_and_report_blob(img, color_name, blob, draw_color):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
    area = w * h
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2

    img.draw_rect(x, y, w, h, draw_color, 2)
    img.draw_cross(cx, cy, draw_color, 10, 2)
    img.draw_string(x, max(0, y - 16),
                    "%s d(%d,%d)" % (color_name, dx, dy), draw_color)

    return "BLOB,%s,%d,%d,%d,%d,%d,%d,%d" % (color_name, dx, dy, cx, cy, w, h, area)


def run_qr(img, frame_id):
    qrcodes = img.find_qrcodes()
    if not qrcodes:
        img.draw_string(0, 0, "QR: none", image.COLOR_RED)
        if frame_id % 30 == 0:
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

    if frame_id % 10 == 0:
        uart_write("QR,%s" % payload)


def run_one_blob(img, color_name, frame_id):
    threshold, draw_color = COLOR_CONFIGS[color_name]
    blob = find_largest_blob(img, threshold)
    if blob is None:
        img.draw_string(0, 0, "BLOB %s: none" % color_name, image.COLOR_RED)
        if frame_id % 20 == 0:
            uart_write("NONE,BLOB,%s" % color_name)
        return
    report = draw_and_report_blob(img, color_name, blob, draw_color)
    if frame_id % 10 == 0:
        uart_write(report)


def run_all_blobs(img, frame_id):
    found = False
    for color_name in ("RED", "GREEN", "BLUE"):
        threshold, draw_color = COLOR_CONFIGS[color_name]
        blob = find_largest_blob(img, threshold)
        if blob is None:
            continue
        found = True
        draw_and_report_blob(img, color_name, blob, draw_color)
        if frame_id % 10 == 0:
            uart_write(draw_and_report_blob(img, color_name, blob, draw_color))
    if not found and frame_id % 20 == 0:
        uart_write("NONE,BLOB,ALL")


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
    if frame_id % 10 == 0:
        uart_write("LINE,%d,%d" % (dx, theta_out))


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("Vision mode test started (UART bidirectional).")
print("MODE =", MODE)

while not app.need_exit():
    uart_read_cmds()

    img = cam.read()
    img.draw_cross(WIDTH // 2, HEIGHT // 2, image.COLOR_WHITE, 6, 1)
    img.draw_string(0, HEIGHT - 16, "MODE:%s" % MODE, image.COLOR_WHITE)

    if MODE == "QR":
        run_qr(img, frame_id)
    elif MODE == "BLOB_RED":
        run_one_blob(img, "RED", frame_id)
    elif MODE == "BLOB_GREEN":
        run_one_blob(img, "GREEN", frame_id)
    elif MODE == "BLOB_BLUE":
        run_one_blob(img, "BLUE", frame_id)
    elif MODE == "BLOB_ALL":
        run_all_blobs(img, frame_id)
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
