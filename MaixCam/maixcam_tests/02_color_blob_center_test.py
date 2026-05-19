from maix import app, camera, display, image


WIDTH = 320
HEIGHT = 240

# 这些是快速测试的更广泛的起始阈值
# 使用 MaixCam 的内置 Find Blob 应用程序根据您的真实对象调整它们
COLOR_CONFIGS = [
    ("RED", [0, 90, 35, 90, -10, 90], image.COLOR_RED),
    ("GREEN", [0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    ("BLUE", [0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
]

# 设置像素和区域阈值，用于过滤小的噪声
PIXELS_THRESHOLD = 350  # 最少像素数量阈值
AREA_THRESHOLD = 350    # 最少区域面积阈值


def find_largest_blob(img, threshold):
    blobs = img.find_blobs(
        [threshold],                            # 颜色阈值
        pixels_threshold=PIXELS_THRESHOLD,      # 最少像素数量阈值
        area_threshold=AREA_THRESHOLD,          # 最少区域面积阈值
        merge=True,                             # 合并相邻的区域
    )
    # 如果没有找到任何色块，返回面积最大的色块（面积 = w * h）
    if not blobs:
        return None
    return max(blobs, key=lambda b: b[2] * b[3])

# 初始化摄像头和显示器
cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("颜色中心测试开始.")
print("输出格式: COLOR,cx,cy,w,h,area,dx,dy")

while not app.need_exit():
    img = cam.read()
    img_cx = WIDTH // 2     # 计算图像中心X坐标
    img_cy = HEIGHT // 2    # 计算图像中心Y坐标
    found = []

    # 在图像上绘制白色十字线，用于参考
    img.draw_cross(img_cx, img_cy, image.COLOR_WHITE, 6, 1)

    for name, threshold, draw_color in COLOR_CONFIGS:
        # 查找当前颜色的最大色块
        blob = find_largest_blob(img, threshold)
        if blob is None:
            continue

        # 提取色块的坐标、宽度、高度、面积、中心坐标、与图像中心的偏移量
        x, y, w, h = blob[0], blob[1], blob[2], blob[3]
        cx = x + w // 2
        cy = y + h // 2
        area = w * h
        dx = cx - img_cx
        dy = cy - img_cy

        # 保留盒子以供调试，控制器主要需要cx/cy/dx/dy
        img.draw_rect(x, y, w, h, draw_color, 2)        # 绘制色块矩形框
        img.draw_cross(cx, cy, draw_color, 10, 2)        # 绘制色块中心十字线
        # 在色块上方显示颜色名称和中心坐标
        img.draw_string(x, max(0, y - 16), "%s (%d,%d)" % (name, cx, cy), draw_color)

        found.append("%s,%d,%d,%d,%d,%d,%d,%d" % (name, cx, cy, w, h, area, dx, dy))

    # 每10帧打印一次结果
    if frame_id % 10 == 0:
        if found:
            for item in found:
                print(item)
        else:
            print("NONE")

    frame_id += 1
    disp.show(img)
