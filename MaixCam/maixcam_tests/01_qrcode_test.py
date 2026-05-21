from maix import app, camera, display, image, time
import sys

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

cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()

last_payload = None
frame_id = 0

print("QR code test started (with dx,dy center offset).")

while not app.need_exit():
    img = cam.read()
    qrcodes = img.find_qrcodes()

    if qrcodes:
        for qr in qrcodes:
            payload = qr.payload()
            corners = qr.corners()

            for i in range(4):
                x1, y1 = corners[i][0], corners[i][1]
                x2, y2 = corners[(i + 1) % 4][0], corners[(i + 1) % 4][1]
                img.draw_line(x1, y1, x2, y2, image.COLOR_RED, 2)

            img.draw_string(qr.x(), max(0, qr.y() - 16), payload, image.COLOR_RED)

            if payload != last_payload or frame_id % 30 == 0:
                uart_write("QR,%s" % payload)
                last_payload = payload
    else:
        if frame_id % 60 == 0:
            uart_write("NONE,QR")
        img.draw_string(0, 0, "QR: none", image.COLOR_RED)

    frame_id += 1
    disp.show(img)
