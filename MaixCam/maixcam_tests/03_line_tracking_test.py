from maix import app, camera, display, image


# MaixVision 直线/巡线测试
# 说明：
# get_regression() 不是“看见任意直线就识别”，它是先按颜色阈值筛选像素，
# 再对这些像素拟合一条直线。所以如果阈值或 ROI 不对，它会拟合到桌面边缘、
# 笔记本边缘、阴影等错误目标。

WIDTH = 320
HEIGHT = 240

# 可选模式：
#   "DARK_LINE"  黑色胶带/深色线
#   "GREEN_LINE" 绿色线
#   "GRAY_ROAD"  灰色车道区域
MODE = "DARK_LINE"

THRESHOLDS = {
    # 深色线：适合黑胶带、深灰线。L 上限越低，越只看暗处。
    "DARK_LINE": [0, 35, -35, 35, -35, 35],

    # 绿色线：如果你用绿色胶带/绿色激光线，可切换到这个。
    "GREEN_LINE": [0, 100, -128, -8, -30, 100],

    # 灰色车道：适合检测一整块灰色道路区域，不适合细黑线。
    "GRAY_ROAD": [30, 70, -10, 15, -10, 10],
}

LINE_THRESHOLD = THRESHOLDS[MODE]

# ROI 格式：[x, y, w, h]
# 不建议直接用最底部整块区域，因为画面边缘/桌面边缘很容易被拟合成直线。
# 如果你的线在画面更下方，可以把 y 调大；如果线在中间，把 y 调小。
ROI = [20, 100, 280, 100]
# ROI = [0, 0, WIDTH, HEIGHT]

# 候选色块太小通常是噪声。
BLOB_PIXELS_THRESHOLD = 30
BLOB_AREA_THRESHOLD = 30
REGRESSION_AREA_THRESHOLD = 80

# 是否显示阈值候选区域。黄色框越多，说明阈值匹配到的东西越多。
SHOW_CANDIDATES = True


def normalize_theta(theta):
    # Maix/OpenMV 风格的 theta 不是直接用于控制的车头偏角，这里简单转成
    # -90 到 90 附近的角度，方便观察。
    if theta > 90:
        return 270 - theta
    return 90 - theta


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("Line tracking test started.")
print("MODE =", MODE)
print("ROI =", ROI)
print("LINE_THRESHOLD =", LINE_THRESHOLD)

while not app.need_exit():
    img = cam.read()

    # 蓝色框：当前真正参与检测的区域。
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

    # 对阈值筛选后的像素做线性回归。
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

        img.draw_string(0, 0, "theta=%d rho=%d" % (theta_out, rho), image.COLOR_GREEN)

        if frame_id % 15 == 0:
            print("LINE,%d,%d" % (theta_out, rho))
    else:
        img.draw_string(0, 0, "LINE: none", image.COLOR_RED)
        if frame_id % 30 == 0:
            print("NONE,LINE")

    frame_id += 1
    disp.show(img)
