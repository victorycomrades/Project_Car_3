import os


script_path = "/root/maixcam_tests/maixcam_uart_vision.py"
if not os.path.exists(script_path):
    script_path = "maixcam_tests/maixcam_uart_vision.py"

with open(script_path, "r") as f:
    exec(f.read())
