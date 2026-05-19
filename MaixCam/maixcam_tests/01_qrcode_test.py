from maix import app, camera, display, image


WIDTH = 320
HEIGHT = 240


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()

last_payload = None
frame_id = 0

print("二维码测试​​开始，在 MaixCam 前面放一个二维码.")

while not app.need_exit():
    img = cam.read()
    qrcodes = img.find_qrcodes()    # 在图像中查找所有二维码

    if qrcodes:
        for qr in qrcodes:
            payload = qr.payload()       # 获取二维码的内容
            corners = qr.corners()       # 获取二维码的四个角点坐标

            # 在图像上绘制二维码的边框
            for i in range(4):
                x1, y1 = corners[i][0], corners[i][1]
                x2, y2 = corners[(i + 1) % 4][0], corners[(i + 1) % 4][1]
                img.draw_line(x1, y1, x2, y2, image.COLOR_RED, 2)

            # 在图像上绘制二维码的内容
            img.draw_string(qr.x(), max(0, qr.y() - 16), payload, image.COLOR_RED)

            # 避免重复打印相同二维码
            if payload != last_payload or frame_id % 30 == 0:
                print("QR:", payload)
                last_payload = payload
    else:
        if frame_id % 60 == 0:
            print("QR: none")
        img.draw_string(0, 0, "QR: none", image.COLOR_RED)

    frame_id += 1
    disp.show(img)
