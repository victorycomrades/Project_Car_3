from maix import app, camera, display, image


# MaixVision 色环圆心测试：多真实圆融合版
# 注意：这是独立测试文件，不修改 05_color_ring_test.py。
#
# 思路：
# 1. 先用颜色阈值 find_blobs 粗略找到某个颜色的色环区域；
# 2. 再只在这个小 ROI 内用 find_circles 检测多个真实圆；
# 3. 多个同心圆的圆心做加权平均，得到更稳定的中心点；
# 4. 最后才绘制检测到的圆和融合后的中心点，避免绘制内容干扰本帧识别。

WIDTH = 320
HEIGHT = 240

# 可选："ALL", "RED", "GREEN", "BLUE"
# 调试时建议先单色，例如 TARGET = "RED"。
TARGET = "ALL"

# 全局搜索区域。相机和放置区固定后，建议缩小 ROI，速度和稳定性都会更好。
# 格式：[x, y, w, h]
ROI = [0, 0, WIDTH, HEIGHT]

# LAB 颜色阈值，格式：[L_min, L_max, A_min, A_max, B_min, B_max]
# 这组是基础值，实际最好用 MaixCam 自带 Find Blobs 工具重新标定。
RING_COLORS = {
    "RED": {
        "threshold": [0, 95, 25, 90, -10, 90],
        "draw": image.COLOR_RED,
    },
    "GREEN": {
        "threshold": [0, 95, -120, -10, -20, 90],
        "draw": image.COLOR_GREEN,
    },
    "BLUE": {
        "threshold": [0, 95, -20, 80, -128, -20],
        "draw": image.COLOR_BLUE,
    },
}

# find_blobs 参数：用于粗定位色环所在区域。
# 色环线条较细、可能是虚线，所以阈值不要太大。
PIXELS_THRESHOLD = 20       # 像素阈值：最小有效像素数
AREA_THRESHOLD = 20         # 区域阈值：最小有效面积
MERGE_MARGIN = 26           # 合并间距：相邻blob合并的最大距离

# 粗定位候选区域过滤。
MIN_RING_SIZE = 18          # 色环最小宽度/高度
MAX_RING_SIZE = 210          # 色环最大宽度/高度
MIN_SQUARE_RATIO = 55      # min(w, h) / max(w, h) * 100
MAX_INK_DENSITY = 75       # colored_pixels / bbox_area * 100

# find_circles 参数：用于在小 ROI 内检测多个真实圆。
# 如果卡顿或内存报错，优先增大 CIRCLE_STRIDE、增大 CIRCLE_R_STEP、缩小 ROI。
USE_MULTI_CIRCLE = True         # 是否使用多圆检测
CIRCLE_THRESHOLD = 1900         # 圆检测阈值：越大越严格
CIRCLE_STRIDE = 3               # 霍夫变换步长：越大越快但精度降低
CIRCLE_R_STEP = 2               # 半径步长：越大越快但精度降低
CIRCLE_MARGIN_XY = 7            # 圆心间距（X/Y方向圆心间距阈值）：大于此值的圆心会被合并
CIRCLE_MARGIN_R = 4             # 半径间距：大于此值的圆径会被合并为一个圆
MAX_DRAW_CIRCLES = 8            # 最大绘制圆数：超过此值的圆会被合并

# 多圆融合过滤：只保留圆心离粗定位中心不太远的圆。
CENTER_GATE_PIXELS = 22         # 中心点距离门限：大于此值的圆会被合并
MIN_VALID_CIRCLES = 2           # 最小有效圆数：保留此数量的圆心

# 平滑系数：手持调试建议 1.0，不然中心会有滞后；固定安装后可改 0.35。
SMOOTH_ALPHA = 1.0              # 中心点平滑系数：越大越稳定，越小越响应快
smooth_centers = {}             # 存储各颜色的平滑中心点


def wanted_colors():
    """返回需要检测的颜色列表"""
    if TARGET == "ALL":
        return ["RED", "GREEN", "BLUE"]
    if TARGET in RING_COLORS:
        return [TARGET]
    raise ValueError("TARGET must be ALL, RED, GREEN, or BLUE")


def clamp_roi(x, y, w, h):
    """限制ROI区域在图像范围内"""
    if x < 0:               # 处理左边界超出
        w += x
        x = 0
    if y < 0:               # 处理上边界超出
        h += y
        y = 0
    if x + w > WIDTH:       # 处理右边界超出
        w = WIDTH - x
    if y + h > HEIGHT:      # 处理下边界超出
        h = HEIGHT - y
    return [x, y, max(1, w), max(1, h)]     # 确保宽高至少为1


def density_percent(blob):
    """计算色块的像素密度百分比"""
    area = blob.w() * blob.h()      # 计算包围盒面积
    if area <= 0:
        return 100
    return blob.pixels() * 100 // area  # 返回密度百分比


def square_ratio(blob):
    """计算色块的方形比例（宽高比）"""
    w = blob.w()
    h = blob.h()
    if w <= 0 or h <= 0:
        return 0
    return min(w, h) * 100 // max(w, h)  # 返回方形比例百分比


def is_ring_like(blob):
    """判断色块是否像色环形状"""
    w = blob.w()
    h = blob.h()
    if w < MIN_RING_SIZE or h < MIN_RING_SIZE:      # 尺寸过小
        return False
    if w > MAX_RING_SIZE or h > MAX_RING_SIZE:      # 尺寸过大
        return False
    if square_ratio(blob) < MIN_SQUARE_RATIO:       # 形状不够方正
        return False

    # 色环是空心细线，bbox 内的有色像素密度不应该太高。
    # 这个过滤可以排除实心色块，但不要设得太低，否则虚线圆容易被过滤。
    if density_percent(blob) > MAX_INK_DENSITY:     # 密度过高
        return False

    return True      # 所有条件都满足，判断为色环


def ring_score(blob):
    """计算色环得分（用于选择最佳候选）"""
    size_score = blob.w() * blob.h()            # 尺寸得分
    square_score = square_ratio(blob) * 40      # 方形得分
    density_score = max(0, 80 - density_percent(blob)) * 10 # 密度反向得分
    return size_score + square_score + density_score    # 总得分


def find_ring_blob(img, threshold):
    """在图像中查找色环blob"""
    blobs = img.find_blobs(
        [threshold],                            # 颜色阈值
        roi=ROI,                                # 搜索区域
        pixels_threshold=PIXELS_THRESHOLD,      # 像素阈值
        area_threshold=AREA_THRESHOLD,          # 区域阈值
        merge=True,                             # 是否合并相邻的blob
        margin=MERGE_MARGIN,                    # 合并间距
    )

    candidates = []         # 存储候选色环
    for blob in blobs:
        if is_ring_like(blob):
            candidates.append(blob)      # 存储候选色环

    if not candidates:
        return None
    return max(candidates, key=ring_score)  # 返回得分最高的候选色环


def blob_fallback(blob):
    """当霍夫圆检测失败时的备用方案"""
    # fallback 只用于霍夫圆失败时兜底，不是最终推荐精定位。
    cx = blob.x() + blob.w() // 2               # 计算中心X坐标
    cy = blob.y() + blob.h() // 2               # 计算中心Y坐标
    radius = max(blob.w(), blob.h()) // 2       # 计算半径
    score = ring_score(blob)                    # 计算得分
    return cx, cy, radius, score, "BLOB"        # 返回备用方案的圆心、半径、得分和来源


def circle_roi_from_blob(blob):
    """从blob生成圆检测的ROI区域"""
    # 在粗定位 bbox 外扩一点，让霍夫圆能看到完整圆边缘。
    pad = 10
    return clamp_roi(
        blob.x() - pad,         # 左上角X坐标
        blob.y() - pad,         # 左上角Y坐标
        blob.w() + pad * 2,     # 宽度（左右各扩展pad）
        blob.h() + pad * 2,     # 高度（上下各扩展pad）
    )


def detect_real_circles(img, blob):
    """在指定区域内检测真实的圆形"""
    roi = circle_roi_from_blob(blob)        # 获取圆检测ROI
    max_side = max(roi[2], roi[3])          # 获取ROI最大边长
    min_side = min(roi[2], roi[3])          # 获取ROI最小边长

    # 根据 ROI 自动估计半径范围。范围越窄，越快、越不容易误检。
    r_min = max(6, min_side // 5)
    r_max = max(r_min + 2, max_side // 2 + 4)

    try:
        circles = img.find_circles(   # 霍夫圆检测
            roi=roi,                    # 搜索区域
            x_stride=CIRCLE_STRIDE,     # X方向步长
            y_stride=CIRCLE_STRIDE,     # Y方向步长
            threshold=CIRCLE_THRESHOLD, # 检测阈值
            x_margin=CIRCLE_MARGIN_XY,  # X方向圆心间距
            y_margin=CIRCLE_MARGIN_XY,  # Y方向圆心间距
            r_margin=CIRCLE_MARGIN_R,   # 半径间距
            r_min=r_min,                # 最小半径
            r_max=r_max,                # 最大半径
            r_step=CIRCLE_R_STEP,       # 半径步进
        )
    except Exception as err:
        print("WARN,FIND_CIRCLES_FAIL,%s" % err)
        return []

    return circles


def fuse_circle_centers(circles, fallback):
    """融合多个圆的中心点，获得更稳定的位置"""
    if not circles:
        return fallback, []     # 没有圆则返回fallback

    bx, by, _, _, _ = fallback  # 获取fallback中心点
    selected = []               # 存储筛选后的圆

    for c in circles:
        dx = c.x() - bx
        dy = c.y() - by
        if dx * dx + dy * dy <= CENTER_GATE_PIXELS * CENTER_GATE_PIXELS:
            selected.append(c)

    # 如果门控太严格导致没有圆，退一步使用强度最高的几个圆。
    if not selected:
        selected = sorted(circles, key=lambda item: item.magnitude(), reverse=True)[:3] # 取强度前3个

    weight_sum = 0  # 权重总和
    sx = 0          # 加权X坐标总和
    sy = 0          # 加权Y坐标总和
    sr = 0          # 加权半径总和
    score = 0       # 总得分

    for c in selected:
        weight = c.magnitude()      # 获取圆的强度作为权重
        if weight <= 0:
            weight = 1          # 权重至少为1
        sx += c.x() * weight      # 累加加权X坐标
        sy += c.y() * weight      # 累加加权Y坐标
        sr += c.r() * weight      # 累加加权半径
        score += c.magnitude()    # 累加得分
        weight_sum += weight      # 累加权重

    cx = sx // weight_sum       # 计算平均X坐标
    cy = sy // weight_sum       # 计算平均Y坐标
    radius = sr // weight_sum   # 计算平均半径

    if len(selected) >= MIN_VALID_CIRCLES:              # 有效圆数量足够
        source = "MULTI_HOUGH%d" % len(selected)        # 标记为多圆融合
    else:
        source = "SINGLE_HOUGH%d" % len(selected)       # 标记为单圆或少圆

    return (cx, cy, radius, score, source), selected    # 返回融合结果和选中的圆


def smooth(color_name, cx, cy):
    """对中心点进行平滑处理，减少抖动"""
    if color_name not in smooth_centers:            # 如果该颜色首次出现
        smooth_centers[color_name] = [cx, cy]       # 初始化平滑中心
        return cx, cy

    old_x, old_y = smooth_centers[color_name]                       # 获取上次平滑坐标
    new_x = int(old_x * (1.0 - SMOOTH_ALPHA) + cx * SMOOTH_ALPHA)
    new_y = int(old_y * (1.0 - SMOOTH_ALPHA) + cy * SMOOTH_ALPHA)
    smooth_centers[color_name] = [new_x, new_y]                     # 更新平滑中心
    return new_x, new_y


def detect_color_ring(img, color_name):
    """检测指定颜色的色环"""
    config = RING_COLORS[color_name]                    # 获取颜色配置
    blob = find_ring_blob(img, config["threshold"])      # 查找色环blob
    if blob is None:
        return None

    fallback = blob_fallback(blob)      # 获取备用中心点
    selected_circles = []               # 初始化选中圆列表

    if USE_MULTI_CIRCLE:                            # 如果启用多圆检测
        circles = detect_real_circles(img, blob)    # 检测真实圆
        result, selected_circles = fuse_circle_centers(circles, fallback)
    else:
        result = fallback

    cx, cy, radius, score, source = result  # 解析结果
    cx, cy = smooth(color_name, cx, cy)     # 应用平滑处理
    dx = cx - WIDTH // 2                    # 计算相对于图像中心的X偏移
    dy = cy - HEIGHT // 2                   # 计算相对于图像中心的Y偏移

    report = (
        "RING,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s"
        % (
            color_name,             # 颜色名称
            cx,                     # 中心X坐标
            cy,                     # 中心Y坐标
            dx,                     # X方向偏移
            dy,                     # Y方向偏移
            radius,                 # 半径
            score,                  # 得分
            density_percent(blob),  # 像素密度
            square_ratio(blob),     # 方形比例
            source,                 # 检测来源
        )
    )

    return {  # 返回结果字典
        "color_name": color_name,       # 颜色名称
        "cx": cx,                       # 中心X坐标
        "cy": cy,                       # 中心Y坐标
        "radius": radius,               # 半径
        "draw_color": config["draw"],   # 绘制颜色
        "report": report,               # 报告字符串
        "circles": selected_circles,    # 选中的圆列表
    }


def draw_result(img, result):
    """在图像上绘制检测结果"""
    color_name = result["color_name"]
    cx = result["cx"]                    # 中心X坐标
    cy = result["cy"]                    # 中心Y坐标
    draw_color = result["draw_color"]    # 绘制颜色
    circles = result["circles"]          # 选中的圆列表

    # 只画真实检测到的圆。若霍夫失败，则画一个 fallback 圆，方便知道仍在兜底。
    if circles:
        count = 0
        for c in circles:
            img.draw_circle(c.x(), c.y(), c.r(), draw_color, 2)
            count += 1
            if count >= MAX_DRAW_CIRCLES:   # 限制绘制数量
                break
    else:
        img.draw_circle(cx, cy, result["radius"], draw_color, 1)

    # 颜色名放在圆外侧，中心只保留十字和坐标。
    label_y = max(0, cy - result["radius"] - 16)
    img.draw_string(max(0, cx - 18), label_y, color_name, draw_color)   # 绘制颜色名称
    img.draw_cross(cx, cy, image.COLOR_BLACK, 12, 3)  # 绘制中心十字
    img.draw_string(max(0, cx + 8), max(0, cy - 8), "(%d,%d)" % (cx, cy), image.COLOR_BLACK)     # 绘制坐标


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("RGB color ring multi-circle test started.")
print("TARGET =", TARGET)
print("Output: RING,color,cx,cy,dx,dy,radius,score,density,ratio,source")

while not app.need_exit():
    img = cam.read()
    img.draw_cross(WIDTH // 2, HEIGHT // 2, image.COLOR_WHITE, 6, 1)

    results = []
    for color_name in wanted_colors():
        result = detect_color_ring(img, color_name)
        if result:
            results.append(result)

    # 所有识别结束后再绘制，避免蓝色十字/坐标被当作蓝色目标。
    for result in results:
        draw_result(img, result)

    if frame_id % 8 == 0:
        if results:
            for result in results:
                print(result["report"])
        else:
            print("NONE,RING,%s" % TARGET)

    frame_id += 1
    disp.show(img)
