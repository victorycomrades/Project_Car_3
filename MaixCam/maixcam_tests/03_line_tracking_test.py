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

# 可选模式：
#   "DARK_LINE"  黑色胶带/深色线
#   "GREEN_LINE" 绿色线
#   "GRAY_ROAD"  灰色车道区域
MODE = "DARK_LINE"

THRESHOLDS = {
    "DARK_LINE": [0, 35, -35, 35, -35, 35],
    "GREEN_LINE": [0, 100, -128, -8, -30, 100],
    "GRAY_ROAD": [30, 70, -10, 15, -10, 10],
}

LINE_THRESHOLD = THRESHOLDS[MODE]
ROI = [20, 100, 280, 100]
BLOB_PIXELS_THRESHOLD = 30
BLOB_AREA_THRESHOLD = 30
REGRESSION_AREA_THRESHOLD = 80
SHOW_CANDIDATES = True


def normalize_theta(theta):
    if theta > 90:
        return 270 - theta
    return 90 - theta


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("Line tracking test started (dx = deviation from center line).")
print("MODE =", MODE)

while not app.need_exit():
    img = cam.read()
    img.draw_rect(ROI[0], ROI[1], ROI[2], ROI[3], image.COLOR_BLUE, 1)

    if SHOW_CANDIDATES:
        blobs = img.find_blobs(
            [LINE_THRESHOLD],
            roi=ROI,
            pixels_threshold=BLOB_PIXELS_THRESHOLD,
            area_threshold=BLOB_AREA_THRESHOLD,
            merge=True,
        )
        for blob in blobs:
            img.draw_rect(blob.x(), blob.y(), blob.w(), blob.h(), image.COLOR_YELLOW, 1)

    lines = img.get_regression(
        [LINE_THRESHOLD],
        roi=ROI,
        area_threshold=REGRESSION_AREA_THRESHOLD,
    )

    if lines:
        line = lines[0]
        img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.COLOR_GREEN, 2)

        theta_out = normalize_theta(line.theta())
        rho = line.rho()
        # dx: 车道线相对于画面中心的横向偏差
        dx = rho - WIDTH // 2

        img.draw_string(0, 0, "dx=%d theta=%d" % (dx, theta_out), image.COLOR_GREEN)

        if frame_id % 15 == 0:
            uart_write("LINE,%d,%d" % (dx, theta_out))
    else:
        img.draw_string(0, 0, "LINE: none", image.COLOR_RED)
        if frame_id % 30 == 0:
            uart_write("NONE,LINE")

    frame_id += 1
    disp.show(img)
