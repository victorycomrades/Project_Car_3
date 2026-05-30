from maix import app, time, uart


UART_DEVICES = ("/dev/ttyGS0", "/dev/ttyS0")
BAUDRATE = 115200
MESSAGE = "hello from maixcam\n"


def open_uart():
    last_error = None
    for device in UART_DEVICES:
        try:
            serial = uart.UART(device, BAUDRATE)
            print("UART opened:", device)
            return serial
        except Exception as err:
            last_error = err
            print("UART open failed:", device, err)
    raise RuntimeError("No UART device available: %s" % last_error)


serial = open_uart()

while not app.need_exit():
    serial.write(MESSAGE)
    time.sleep_ms(500)
