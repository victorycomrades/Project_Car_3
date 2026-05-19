import os


script_path = "/root/maixcam_tests/01_qrcode_test.py"
if not os.path.exists(script_path):
    script_path = "maixcam_tests/01_qrcode_test.py"

with open(script_path, "r") as f:
    exec(f.read())
