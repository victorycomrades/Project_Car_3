# 智能物流搬运课程项目

本项目参考 2025 年中国大学生工程实践与创新能力大赛“智能+”赛道中“智能物流搬运赛项”的初赛任务，目标是设计并实现一台能够自主完成物料识别、搬运、放置与返回的智能物流搬运机器人。

## 1. 项目目标

机器人需要在模拟工业物流场景中自主完成以下流程：

1. 从蓝色启停区一键启动。
2. 移动到二维码板前，读取随机任务码。
3. 根据任务码识别红、绿、蓝三种物料的搬运顺序。
4. 从原料区旋转圆盘上按顺序抓取第一批三个物料。
5. 将物料放到机器人自身承载区后运输到粗加工区。
6. 按颜色和任务顺序将物料放置到粗加工区对应区域。
7. 再从粗加工区将物料搬到机器人上，运输到暂存区并放置。
8. 返回原料区，执行第二批三个物料的同样流程。
9. 第二批物料到暂存区时，需要放置在第一批同色物料上，实现码垛。
10. 完成全部任务后，机器人返回启停区。

任务码由两组三位数组成，例如 `123+231`：

- `1` 表示红色物料；
- `2` 表示绿色物料；
- `3` 表示蓝色物料；
- `+` 前表示第一批搬运顺序；
- `+` 后表示第二批搬运顺序。

## 2. 初赛场地与物料

初赛场地为 `2400mm x 2400mm` 的正方形区域，机器人离开启停区后只能在灰色车道上行驶。

主要区域：

- 启停区：蓝色，尺寸 `300mm x 300mm`。
- 原料区：圆形电动转盘，直径约 `300mm`，高度约 `80-100mm`。
- 粗加工区：尺寸约 `580mm x 150mm`，带颜色放置区域和色环。
- 暂存区：尺寸约 `580mm x 150mm`，带颜色放置区域和色环。

初赛物料：

- 红、绿、蓝三种颜色；
- 每种颜色两个；
- 共两批，每批红绿蓝各一个；
- 初赛物料形状相同，只需区分颜色；
- 材料为 3D 打印 ABS；
- 尺寸约 `30-100mm`；
- 重量约 `40-100g`。

## 3. 机器人关键约束

- 最大投影尺寸不超过 `300mm x 300mm`。
- 高度不超过 `400mm`。
- 可以采用可折叠结构，但只能出发后自行展开。
- 只能使用随车装载的一个锂电池。
- 比赛过程中不能更换电池、零部件或电子元器件。
- 比赛过程中不能使用无线通信控制机器人。
- 只能一键启动一次，启动键必须在机器人上且有明确标识。
- 物料跨区域运输时必须放置在机器人承载区，不能由机械爪夹持运输。
- 每次只能抓取一个物料，放到机器人上后才能抓取下一个。
- 机器人每次最多装载三个物料。
- 机器人上部需要有醒目的任务码显示装置，持续显示完整任务信息。

## 4. 功能模块拆分

### 4.1 视觉感知

视觉部分计划基于 MaixCam 实现，主要功能包括：

- 二维码识别：读取任务码，例如 `123+231`。
- 颜色识别：识别红、绿、蓝物料。
- 物料定位：获取物料在图像中的中心位置或抓取参考点。
- 放置区定位：识别粗加工区、暂存区中的颜色区域或色环中心。
- 状态辅助判断：判断物料是否已进入抓取范围、是否放置到位、是否完成码垛。

### 4.2 移动底盘

移动底盘需要完成：

- 灰色车道导航；
- 区域间路径切换；
- 定点停车；
- 原料区、粗加工区、暂存区、二维码区、启停区之间的往返。

可选方案包括：

- 视觉巡线；
- 颜色/边界检测；
- 编码器里程计；
- IMU 辅助转向；
- 固定路径加视觉校正。

### 4.3 机械抓取与承载

机械部分需要完成：

- 从旋转转盘抓取单个物料；
- 将物料放到车载承载区；
- 从车载承载区或粗加工区取出物料；
- 将物料放到指定色环或颜色区域；
- 第二批物料在暂存区完成同色码垛。

### 4.4 任务状态机

推荐使用状态机组织完整流程：

1. `INIT`：初始化摄像头、显示屏、电机、舵机、任务变量。
2. `READ_QR`：读取二维码任务码。
3. `GO_RAW_AREA`：移动到原料区。
4. `PICK_RAW_BATCH`：按任务顺序抓取当前批次物料。
5. `GO_PROCESS_AREA`：移动到粗加工区。
6. `PLACE_PROCESS`：按顺序放置到粗加工区。
7. `PICK_PROCESS`：从粗加工区重新抓取物料。
8. `GO_STORAGE_AREA`：移动到暂存区。
9. `PLACE_STORAGE`：第一批平放，第二批码垛。
10. `NEXT_BATCH`：判断是否进入第二批任务。
11. `RETURN_START`：返回启停区。
12. `DONE`：任务完成。

## 5. 推荐开发流程

1. 单独调通 MaixCam 摄像头取图。
2. 单独调通二维码识别并解析任务码。
3. 单独调通红、绿、蓝颜色识别。
4. 输出物料中心点坐标，完成图像坐标到抓取动作的对应关系。
5. 联调机械爪，实现单个物料抓取。
6. 联调车载承载区，实现“抓取 -> 放到车上 -> 再取出”。
7. 完成单色单物料的“原料区 -> 粗加工区”流程。
8. 扩展到三色第一批完整流程。
9. 加入“粗加工区 -> 暂存区”流程。
10. 加入第二批任务和码垛。
11. 最后进行全流程稳定性和时间优化。

## 6. 后续记录

后续项目讨论中，如果出现关键设计决策、重要调试结论、硬件接口定义、视觉阈值、状态机改动或比赛规则补充，可以继续记录到本文件中。

## 7. MaixCam 视觉方案记录

### 7.1 开发环境与基本模型

MaixCam 推荐使用 MaixPy v4 进行开发。基础视觉程序通常由三部分组成：

1. `camera.Camera()` 初始化摄像头。
2. `cam.read()` 在循环中获取图像。
3. 在同一帧图像上调用二维码、色块、巡线等算法。
4. 使用 `display.Display()` 和 `disp.show(img)` 在屏幕上显示调试画面。

典型结构：

```python
from maix import camera, display, image, app

cam = camera.Camera(320, 240)
disp = display.Display()

while not app.need_exit():
    img = cam.read()

    # 根据当前任务状态选择视觉算法
    # 例如：读二维码、找颜色物料、找车道线

    disp.show(img)
```

官方建议图像处理和 AI 识别场景优先使用较低分辨率，例如 `320x240` 或 `320x224`，这样速度和稳定性更好。

### 7.2 二维码识别

普通二维码识别可使用：

```python
qrcodes = img.find_qrcodes()
for qr in qrcodes:
    payload = qr.payload()
    corners = qr.corners()
```

可用信息：

- `qr.payload()`：二维码内容，例如 `123+231`。
- `qr.corners()`：二维码四个角点坐标。
- `qr.x()`、`qr.y()`：二维码左上角坐标。

MaixPy v4.7.9 及以后还提供 `image.QRCodeDetector()`，可以使用硬件加速二维码检测。在不同时运行其它 NPU 模型时，可以优先考虑它。

### 7.3 颜色识别与物料定位

红、绿、蓝物料识别可使用 `img.find_blobs()`。该方法基于 LAB 颜色阈值找色块。

阈值格式：

```python
[L_MIN, L_MAX, A_MIN, A_MAX, B_MIN, B_MAX]
```

官方示例阈值：

```python
RED_THRESHOLD = [0, 80, 40, 80, 10, 80]
GREEN_THRESHOLD = [0, 80, -120, -10, 0, 30]
BLUE_THRESHOLD = [0, 80, 30, 100, -120, -60]
```

示例：

```python
thresholds = [RED_THRESHOLD, GREEN_THRESHOLD, BLUE_THRESHOLD]
blobs = img.find_blobs(thresholds, pixels_threshold=500)

for blob in blobs:
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
```

调试时应根据实际光照、物料颜色、摄像头角度重新标定 LAB 阈值。MaixCam 自带 `Find Blobs` 应用，可以辅助观察和调试阈值。

### 7.4 巡线与路径辅助

如果后续采用视觉巡线，可使用 `img.get_regression()` 找指定颜色区域或线条的主方向。

返回的线对象可读取：

- `line.x1()`、`line.y1()`、`line.x2()`、`line.y2()`：线段端点。
- `line.theta()`：线方向角。
- `line.rho()`：线到原点距离。

灰色车道识别可能需要现场重新标定阈值，或者用底部 ROI 只看车道局部区域，提高速度和抗干扰能力。

### 7.5 与底盘控制板通信

如果 MaixCam 只负责视觉，底盘和机械臂由 STM32、Arduino 或其它控制板负责，推荐通过 UART 输出结构化识别结果。

MaixPy 串口基础写法：

```python
from maix import uart

serial_dev = uart.UART("/dev/ttyS0", 115200)
serial_dev.write_str("QR:123+231\n")
```

建议后续统一通信协议，例如：

```text
QR,123+231,0,0
BLOB,RED,-6,-18,154,102,38,41,1558
LINE,12,-8
STATE,PICK_RED
```

如果使用非默认串口，需要先用 `pinmap.set_pin_function()` 设置引脚复用。MaixCam/MaixCam-Pro 的 UART0 默认可用引脚通常是 `A16` TX 和 `A17` RX，但 UART0 启动时会输出系统日志，和下位机通信时要忽略开机阶段的数据。

### 7.6 与舵机或补光灯控制

MaixPy 支持 PWM，可用于舵机或补光灯亮度控制。使用 PWM 前也需要通过 `pinmap` 设置引脚功能。

舵机典型参数：

- 频率：`50Hz`
- 常见占空比范围：`2.5% - 12.5%`

如果机械臂控制较复杂，建议由下位机统一控制舵机，MaixCam 只发送视觉结果和任务状态，避免视觉主循环被复杂动作阻塞。

### 7.7 多功能整合思路

官方示例通常每个功能单独写一个 `while` 循环，但项目中应整合成一个主循环和多个函数：

```python
while not app.need_exit():
    img = cam.read()

    if state == "READ_QR":
        task_code = detect_qrcode(img)
    elif state == "FIND_RAW_MATERIAL":
        target = detect_color_blob(img, wanted_color)
    elif state == "LINE_FOLLOW":
        line = detect_line(img)
    elif state == "PLACE_CHECK":
        result = detect_place_area(img)

    send_result_to_controller()
    disp.show(img)
```

整合原则：

- 每一帧只运行当前状态需要的算法，避免所有算法同时跑导致帧率下降。
- 二维码只在 `READ_QR` 状态识别，识别成功后缓存任务码，不必全程重复识别。
- 找物料时使用 ROI 限制搜索区域，例如只看转盘区域或机械爪前方区域。
- 找色块时根据当前任务只找目标颜色，减少误检。
- 对识别结果做连续多帧确认，避免单帧误识别。
- 视觉只输出“看到了什么、在哪里、置信是否足够”，动作决策交给状态机。

### 7.8 MaixVision 单功能验证脚本

当前已在 `maixcam_tests/` 目录下提供 MaixVision 验证脚本：

- `01_qrcode_test.py`：二维码识别，打印二维码内容并在图像上画框。
- `02_color_blob_test.py`：红、绿、蓝色块识别，打印色块中心点和宽高。
- `03_line_tracking_test.py`：直线/灰色车道识别，用于巡线思路验证。
- `04_vision_mode_test.py`：综合模式脚本，通过修改 `MODE` 变量切换识别功能。
- `05_color_ring_test.py`：基础色环中心识别版本，使用颜色阈值粗定位，并在画面上绘制圆心、坐标和调试圆环。
- `06_color_ring_multi_circle_test.py`：独立的多真实圆检测与圆心融合实验版，不修改 `05_color_ring_test.py`。

这些脚本是在 Windows 电脑的 MaixVision 中连接 MaixCam 后运行，代码实际运行在 MaixCam 设备上，不是在 Windows 本地 Python 中运行。

建议验证顺序：

1. 先运行 `01_qrcode_test.py`，确认二维码识别和终端打印正常。
2. 再运行 `02_color_blob_test.py`，确认红绿蓝物料能被识别。
3. 使用 MaixCam 自带 `Find Blobs` 应用校准 LAB 阈值，再回填到脚本中。
4. 有场地样片后运行 `03_line_tracking_test.py`，调试灰色车道或线条阈值。
5. 最后运行 `04_vision_mode_test.py`，模拟后续由 Jetson 下发 `MODE` 后只运行当前视觉任务的结构。

色环识别当前采用“颜色粗定位 + 多真实圆检测 + 圆心融合”的方案：先用颜色阈值找色环大致 ROI，再在小 ROI 内用 `find_circles()` 检测多个真实同心圆，最后对这些圆的圆心做加权平均。

1. 先用 `320x240` 分辨率，避免 MaixCam 快帧缓冲内存不足。
2. 只检测红、绿、蓝三个色环。
3. 调试阈值时建议先单色测试，例如 `TARGET = "RED"`。
4. 画面上绘制的是 `find_circles()` 真实检测到、参与圆心融合的圆，不再按比例伪造同心圆。
5. 若多圆检测失败，程序会退回使用颜色区域几何中心，输出中的 `source` 会显示 `BLOB`。
6. 输出 `RING,颜色,dx,dy,cx,cy,radius,score,density,ratio,source`，其中 `source=MULTI_HOUGHn` 表示融合了 n 个真实圆。`cx, cy` 用于后续几何定位；`dx, dy` 是相对画面中心的默认调试偏差。

色环多圆融合调试结论：

- `find_blobs()` 负责按颜色粗定位，决定是否能找到某个颜色的色环区域。
- `find_circles()` 负责在粗定位得到的小 ROI 内检测真实圆边界，决定能检测出几条同心圆。
- `MULTI_HOUGHn` 表示融合了 n 个真实圆；`SINGLE_HOUGH1` 表示只检测到 1 个圆；`BLOB` 表示霍夫圆失败，退回颜色区域中心。
- 蓝色通常比绿色检测出更多圆，是因为蓝色在当前相机画面中饱和度和对比度更高，LAB 阈值更容易把蓝色线条从纸面背景中分离出来。
- 绿色检测圆少，常见原因是绿色印刷线条偏灰、光照不足、阈值只覆盖了部分绿色像素，导致虚线被分割得更碎，霍夫圆投票不足。
- 内外圈跳变通常是因为每一帧通过阈值和霍夫投票留下来的圆集合不同：有时内圈得票高，有时外圈得票高，最终融合的圆集合变化，显示的圆就会跳。
- 提高稳定性的优先顺序：固定相机和色环距离，缩小 `ROI`，用 Find Blobs 标定三色 LAB 阈值，单色调试 `TARGET`，再调整 `CIRCLE_THRESHOLD`、`CENTER_GATE_PIXELS`、`SMOOTH_ALPHA`。
- 机械臂最终更关心融合后的 `cx, cy` 是否稳定，不必强求每一帧都画出全部内外圆。

直线检测调试结论：

- `03_line_tracking_test.py` 使用 `get_regression()`，它不是直接找几何边缘，而是先按 LAB 阈值筛选像素，再对这些像素做线性拟合。
- 蓝色框是参与检测的 ROI，黄色框是阈值命中的候选区域，绿色线是最终拟合线。
- 如果黄色框出现在桌面边缘、笔记本边缘或阴影上，说明 ROI 或阈值太宽，绿色拟合线会被错误目标带偏。
- 巡线时应让 ROI 只覆盖机器人前方的道路/线条区域，避免把画面边缘和背景边界纳入拟合。

### 7.9 当前视觉脚本职责边界

当前保留两个色环版本，避免一个文件里混入过多实验逻辑：

- `05_color_ring_test.py`：基础版。适合快速验证某个颜色是否能被 `find_blobs()` 粗定位出来，逻辑较简单，便于调 LAB 阈值。
- `06_color_ring_multi_circle_test.py`：实验增强版。适合验证“多真实圆参与圆心融合”是否能提升放置中心稳定性。

两者的区别：

```text
05：颜色阈值 -> 色环区域中心 -> 调试显示
06：颜色阈值 -> 小 ROI -> 霍夫圆检测多个真实圆 -> 圆心门控 -> 半径稳定筛选 -> 加权融合圆心
```

推荐使用顺序：

1. 先用 `05_color_ring_test.py` 调通目标颜色，确认 LAB 阈值能抓到色环。
2. 再用 `06_color_ring_multi_circle_test.py` 验证多圆融合后的 `cx, cy` 是否更稳定。
3. 如果 `06` 输出经常是 `BLOB`，说明霍夫圆没有稳定工作，应先回到 `05` 调阈值和 ROI。
4. 如果 `06` 输出 `MULTI_HOUGHn`，说明已有多个真实圆参与融合，重点观察 `cx, cy` 是否稳定。

### 7.10 霍夫圆检测说明

霍夫圆检测用于从图像边缘中寻找圆。一个圆可以由三个参数描述：

```text
圆心 x
圆心 y
半径 r
```

算法思想是：边缘点对可能的 `(x, y, r)` 进行投票，票数高的位置被认为存在圆。

在本项目中，不建议直接全图霍夫圆检测。原因是 MaixCam 算力和高速帧缓冲有限，全图、多颜色、多半径搜索容易卡顿或内存不足。`06_color_ring_multi_circle_test.py` 的做法是先用颜色阈值粗定位，再在小 ROI 内做霍夫圆检测。

这样做的好处：

- 计算区域更小，速度更快。
- 误检更少。
- 能利用色环的多个同心圆共同估计圆心。
- 单个圆断裂或偏移时，多个圆的加权融合可以降低偶然误差。

### 7.11 关键调试经验

1. MaixVision 里的代码运行在 MaixCam 设备上，不是在 Windows 本地 Python 中运行。
2. `print()` 输出在 MaixVision 终端中查看。
3. 不要在检测某个颜色之前，就在同一帧图像上画该颜色可能识别到的标记。之前出现过“蓝色十字被识别成蓝色目标”的问题，解决方法是先完成所有检测，再统一绘制调试图形。
4. 色块和色环的矩形框主要用于调试；机械臂几何定位真正需要的是 `cx, cy`，闭环微调需要的是目标相对抓取参考点的误差。
5. 色环远距离识别困难是正常现象。色环线条变细后，符合阈值的像素变少，应通过固定相机、缩小 ROI、重新标定 LAB 和必要时提高分辨率解决。
6. 不要默认使用 `480x320 + ALL + 霍夫圆`。这个组合容易触发 MaixCam 快帧缓冲内存不足。优先使用 `320x240`，单色调试，小 ROI。
7. 识别结果应连续多帧确认，不能只相信单帧结果。后续整合到 Jetson 时，应在上位机或 MaixCam 侧做稳定性判断。

### 7.12 视觉输出含义

建议后续统一输出格式：

```text
QR,123+231
BLOB,RED,dx,dy,cx,cy,w,h,area
RING,RED,dx,dy,cx,cy,radius,score,density,ratio,source
LINE,dx,theta
NONE,RING,RED
```

字段解释：

- `cx, cy`：目标中心点在图像中的绝对像素坐标，原点为图像左上角，后续用于几何定位和标定计算。
- `dx, dy`：目标中心相对图像中心的默认偏差，即 `dx = cx - WIDTH/2`、`dy = cy - HEIGHT/2`，适合调试和画面中心闭环。
- 抓取闭环不一定以图像中心为目标。若相机光轴、夹爪中心和抓取点不重合，应使用标定出的抓取参考点 `(GRASP_U, GRASP_V)`，计算 `err_u = cx - GRASP_U`、`err_v = cy - GRASP_V`。
- `radius`：检测到的圆半径或估计半径。
- `source`：色环中心来源，常见值包括 `BLOB`、`SINGLE_HOUGH1`、`MULTI_HOUGHn`。
- `MULTI_HOUGHn`：融合了 n 个真实霍夫圆，通常比单圆或 `BLOB` 更可信。

### 7.13 官方资料链接

- MaixPy 文档首页：https://wiki.sipeed.com/maixpy/en/index.html
- Camera 使用：https://wiki.sipeed.com/maixpy/doc/en/vision/camera.html
- 二维码识别：https://wiki.sipeed.com/maixpy/doc/en/vision/qrcode.html
- 色块识别：https://wiki.sipeed.com/maixpy/doc/en/vision/find_blobs.html
- 巡线识别：https://wiki.sipeed.com/maixpy/doc/en/vision/line_tracking.html
- UART 串口：https://wiki.sipeed.com/maixpy/doc/en/peripheral/uart.html
- PWM 控制：https://wiki.sipeed.com/maixpy/doc/en/peripheral/pwm.html

## 8. Jetson Nano + STM32 + MaixCam 系统架构建议

当前项目硬件结构：

- 上位机：Jetson Nano，Ubuntu 20.04，运行 ROS。
- 下位机：STM32F407VET6，负责实时电机、舵机、编码器、传感器等控制。
- 视觉模块：MaixCam，负责摄像头图像采集和轻量视觉识别。

推荐架构为：

```text
MaixCam  --UART/USB串口-->  Jetson Nano / ROS  --UART/USB串口-->  STM32F407
```

职责划分：

- MaixCam：做前端视觉传感器，只输出识别结果。
- Jetson Nano：做任务决策、状态机、路径规划、ROS 节点调度和数据融合。
- STM32：做底层实时控制，包括电机闭环、舵机动作、机械爪时序、编码器读取和安全保护。

不推荐让 MaixCam 直接控制 STM32 作为主方案，除非项目为了简化系统，决定让 Jetson Nano 不参与实时任务流程。原因是 Jetson Nano 已经运行 ROS，更适合做全局任务调度；MaixCam 更适合做“视觉协处理器”，避免让它同时承担复杂流程控制。

### 8.1 推荐通信方式

MaixCam 与 Jetson Nano：

- 推荐使用 USB 串口或 TTL UART。
- MaixCam 输出二维码、颜色目标、目标中心点、巡线偏差等轻量结果。
- Jetson Nano 上写 ROS 节点读取串口，并发布为 ROS topic。

Jetson Nano 与 STM32：

- 推荐使用 USB 转串口或 TTL UART。
- Jetson Nano 根据状态机向 STM32 下发运动和机械动作命令。
- STM32 返回底盘状态、动作完成标志、编码器数据、错误码等。

### 8.2 推荐 ROS 节点划分

```text
maixcam_bridge_node
    读取 MaixCam 串口数据
    发布 /vision/qrcode
    发布 /vision/blob
    发布 /vision/line

task_manager_node
    维护比赛任务状态机
    解析任务码
    决定当前要找什么颜色、去哪个区域、执行哪个动作
    订阅视觉结果和 STM32 状态
    发布底盘/机械臂命令

stm32_bridge_node
    与 STM32 串口通信
    发布 /robot/base_state
    发布 /robot/action_done
    订阅 /cmd_vel 或自定义动作命令
```

### 8.3 MaixCam 输出协议建议

MaixCam 不直接输出复杂决策，只输出结构化感知结果。

二维码：

```text
QR,123+231
```

颜色物料：

```text
BLOB,RED,-6,-18,154,102,38,41,1558
BLOB,GREEN,-40,-22,120,98,34,36,1224
BLOB,BLUE,28,-15,188,105,40,39,1560
```

字段含义：

```text
BLOB,颜色,dx,dy,中心x,中心y,宽度,高度,面积
```

巡线或车道偏差：

```text
LINE,dx,theta
```

字段含义：

```text
dx     图像中心与车道中心的横向偏差
theta  车道线或中心线角度
```

识别失败：

```text
NONE,QR
NONE,BLOB,RED
NONE,LINE
```

### 8.4 Jetson 给 MaixCam 的控制指令

为了提高识别速度，Jetson 可以告诉 MaixCam 当前需要运行哪个视觉任务。

```text
MODE,QR
MODE,BLOB,RED
MODE,BLOB,GREEN
MODE,BLOB,BLUE
MODE,LINE
MODE,IDLE
```

这样 MaixCam 每一帧只运行当前需要的算法，不必同时二维码、色块、巡线全部跑。

### 8.5 Jetson 给 STM32 的控制指令

建议 STM32 接收“动作级命令”，而不是让 ROS 直接控制每个 PWM 细节。

示例：

```text
MOVE_TO,QR_AREA
MOVE_TO,RAW_AREA
MOVE_TO,PROCESS_AREA
MOVE_TO,STORAGE_AREA
MOVE_TO,START_AREA
ALIGN,BLOB,154,102
PICK
PUT_ON_BOARD,1
PLACE,PROCESS,RED
PLACE,STORAGE,RED,STACK
STOP
```

STM32 执行动作后返回：

```text
DONE,MOVE_TO,RAW_AREA
DONE,PICK
ERR,PICK_TIMEOUT
```

### 8.6 为什么不建议 MaixCam 直接接 STM32 做主控

MaixCam 直接连接 STM32 的方案也可行：

```text
MaixCam --UART--> STM32
```

优点：

- 链路短；
- 延迟低；
- 系统简单；
- 适合只做固定流程的小车。

缺点：

- ROS 系统价值发挥不出来；
- 任务状态机、路径规划、日志记录和调试不方便；
- 后续想融合编码器、IMU、地图、上层策略时扩展性差；
- MaixCam 既做视觉又做流程控制，程序容易变复杂。

因此本项目更推荐：

```text
MaixCam 做视觉协处理器
Jetson Nano 做 ROS 上位机和任务大脑
STM32 做实时运动控制器
```

### 8.7 推荐实施顺序

1. 先让 MaixCam 单独完成二维码识别和颜色识别，并在屏幕上显示识别框。
2. MaixCam 通过串口向电脑或 Jetson 输出 `QR`、`BLOB` 等文本数据。
3. Jetson 上写 ROS 串口节点，把 MaixCam 数据转成 ROS topic。
4. STM32 单独完成底盘运动、机械爪动作和串口命令解析。
5. Jetson 上写 `task_manager_node`，把视觉结果和 STM32 动作串成完整状态机。
6. 最后再做全流程联调。

## 9. 当前待办与调试清单

### 9.1 重新标定 LAB 阈值

需要做：用 MaixCam 自带 `Find Blobs` 工具分别标定红、绿、蓝物料和红、绿、蓝色环的 LAB 阈值。

为什么：当前代码中的阈值是通用起始值，不一定适合实际打印色环、实际物料、现场光照和相机角度。绿色、蓝色识别不稳定，大概率就是阈值只覆盖了部分有效像素，导致 `find_blobs()` 粗定位不完整，后续 `find_circles()` 也只能检测到少数圆。

怎么做：

1. 固定相机高度、角度和光照。
2. 打开 MaixCam 自带 `Find Blobs` 应用。
3. 分别对红、绿、蓝色环线条取样。
4. 记录 `[L_min, L_max, A_min, A_max, B_min, B_max]`。
5. 回填到 `05_color_ring_test.py` 和 `06_color_ring_multi_circle_test.py` 的 `RING_COLORS`。
6. 对物料颜色也重复一遍，回填到 `02_color_blob_center_test.py` 或后续整合代码。

### 9.2 固定相机并缩小 ROI

需要做：不要手持测试最终参数，固定 MaixCam 与色环的距离和角度，然后把 `ROI` 缩小到色环可能出现的区域。

为什么：全图检测会把纸张边缘、桌面、阴影和其它色环纳入搜索范围，导致误检和跳变。缩小 ROI 能提升速度，降低内存压力，也能让霍夫圆检测更稳定。

怎么做：

1. 先用整图 `ROI = [0, 0, WIDTH, HEIGHT]` 找到色环中心大概坐标。
2. 根据输出的 `cx, cy, radius` 设置 ROI，例如中心 `(145, 117)`、半径 `60`，可以设置：
   ```python
   ROI = [65, 35, 160, 160]
   ```
3. 保证目标色环完整落在蓝色 ROI 框内。
4. 后续机械臂放置时，只在放置区附近打开色环识别，不需要远距离全图搜索。

### 9.3 调整 `SMOOTH_ALPHA`

需要做：手持测试和固定安装使用不同平滑系数。

为什么：`SMOOTH_ALPHA` 越小，中心点越稳，但响应越慢；手持相机时会觉得十字滞后。固定安装后，相机不动，使用较小平滑可以抑制检测抖动。

推荐值：

```python
# 手持调试
SMOOTH_ALPHA = 1.0

# 固定安装
SMOOTH_ALPHA = 0.35
```

### 9.4 调整多圆检测稳定性

需要做：在 `06_color_ring_multi_circle_test.py` 中观察 `source` 字段和画面圆环，调节霍夫圆参数。

为什么：`MULTI_HOUGHn` 表示多个真实圆参与融合，是理想状态；`SINGLE_HOUGH1` 说明只检测到一个圆；`BLOB` 说明霍夫圆失败，只能退回颜色区域中心。内外圈跳变通常是每帧参与融合的半径集合不同。

当前已加入的稳定策略：

- `SMOOTH_ALPHA = 0.35`：固定安装时让中心点更稳。
- `RADIUS_STABILITY_ENABLE = True`：记录上一帧中位半径，优先保留半径接近的一组圆。
- `RADIUS_GATE_PIXELS = 16`：允许半径变化的范围。

可调参数：

```python
CIRCLE_THRESHOLD = 1900      # 越大越严格，误检少但可能漏圆
CIRCLE_STRIDE = 3            # 越大越快但精度降低
CIRCLE_R_STEP = 2            # 越大越快但半径搜索更粗
CENTER_GATE_PIXELS = 22      # 圆心离粗定位中心多远以内才参与融合
RADIUS_GATE_PIXELS = 16      # 半径离上一帧中位半径多远以内才优先保留
```

调参方向：

- 圆太少：降低 `CIRCLE_THRESHOLD`，或重新标定 LAB 阈值。
- 误检太多：提高 `CIRCLE_THRESHOLD`，缩小 `ROI`。
- 内外圈跳变：减小 `RADIUS_GATE_PIXELS`，或固定相机后降低 `SMOOTH_ALPHA`。
- 运行卡顿：增大 `CIRCLE_STRIDE` 或 `CIRCLE_R_STEP`，并缩小 `ROI`。

### 9.5 直线/巡线测试

需要做：根据实际赛道颜色选择 `03_line_tracking_test.py` 中的 `MODE` 和 `ROI`。

为什么：`get_regression()` 是对阈值命中的像素做线性拟合，不是直接识别任意几何边缘。ROI 太大时，桌面边缘、纸张边缘和阴影会把拟合线带偏。

怎么做：

1. 黑线或深色线使用：
   ```python
   MODE = "DARK_LINE"
   ```
2. 绿色线使用：
   ```python
   MODE = "GREEN_LINE"
   ```
3. 灰色车道区域使用：
   ```python
   MODE = "GRAY_ROAD"
   ```
4. 调整 `ROI = [x, y, w, h]`，只覆盖机器人前方的线或车道，不要包含画面边缘和大面积背景。

### 9.6 后续整合方向

需要做：等单功能稳定后，把二维码、物料色块、色环中心、直线检测统一成 MaixCam 视觉节点。

推荐原则：

- 每一帧只运行当前状态需要的算法。
- 二维码只在 `READ_QR` 状态识别。
- 物料抓取时只识别当前目标颜色。
- 放置阶段才打开色环识别。
- MaixCam 输出 `QR`、`BLOB`、`RING`、`LINE` 结果给 Jetson，Jetson 决定下一步动作。

## 10. 项目目录结构与硬件通信梳理

### 10.1 目录总览

```text
Project/
├── 8KTM-ROS/          STM32F407VET6 底层固件（CubeMX + HAL）
├── MaixCam/           MaixCam 视觉代码（MaixPy v4 / MaixVision）
├── xrobot2_ws/        老师给的 ROS1 工作空间 v2（完整功能包集合）
├── xrobot3_ws/        老师给的 ROS1 工作空间 v3（串口通信 + 底盘控制）
├── xrobot4_ws/        Jetson Nano 上位机工作空间（我们的代码）
├── README.md          项目说明（本文件）
└── 智能+赛道命题与运行（正式版）.pdf
```

### 10.2 STM32 底层固件（8KTM-ROS/MDK）

- **MCU**: STM32F407VET6
- **开发环境**: STM32CubeMX + HAL 库，支持 RT-Thread OS 可选（`SYS_SUPPORT_OS`）

**串口配置**:

| 串口 | TX Pin | RX Pin | 波特率 | 备注 |
|------|--------|--------|--------|------|
| USART1 | PA9 | PA10 | 1,000,000 | 主通信口，`Uart_Send()`/`Uart_Read()` 默认绑定 |
| USART2 | PD5 | PD6 | 115200 | |
| USART3 | PB10 | PB11 | 115200 | DMA 循环接收 |
| UART4 | PC10 | PC11 | 115200 | DMA 收发 |
| UART5 | PC12 | PD2 | 19200 | 9 位数据位 + 奇校验，仅 TX |
| USART6 | PC6 | PC7 | 115200 | |

**其他外设**: CAN1、CAN2、SPI1、ADC1、TIM1/2/4/5/8（PWM/编码器）

**当前状态**: `main.c` 初始化了所有外设，主循环 `while(1)` 为空，业务逻辑尚未编写。

### 10.3 xrobot2_ws（老师 ROS 工作空间 v2）

ROS1 工作空间，包结构：

| 包 | 功能 |
|----|------|
| `xrobot_arm/` | 机械臂控制（Python，含 jetarm 子模块） |
| `xrobot_cv/` | 计算机视觉 |
| `xrobot_description/` | 机器人 URDF 描述文件 |
| `xrobot_driver/` | 底盘电机驱动 |
| `xrobot_msgs/` | 自定义 ROS 消息定义 |
| `xrobot_navigation/` | 导航功能包 |
| `xrobot_slam/` | SLAM 功能包 |
| `xrobot_teleop/` | 遥控/遥操作 |
| `xrobot_tools/` | 工具包 |
| `third_packages/` | 第三方包（astra_camera、ydlidar、hector_slam、gmapping、joystick 等） |

### 10.4 xrobot3_ws（老师 ROS 工作空间 v3）

核心包与 v2 有所不同，重点是 **串口通信 + 底盘控制**：

**`serial_comm/` — Jetson ↔ STM32 串口通信协议**：

- 传感器数据帧（STM32 → Jetson）：帧头 `"SD"`，34 字节，含巡线传感器×4、超声波×4、陀螺仪×3、加速度×3、欧拉角×3，CRC16-IBM 校验
- 控制指令帧（Jetson → STM32）：帧头 `"ZD"`，含 4 路电机速度 + 5 路关节角度 + 复位标志，CRC16-IBM 校验
- 发布 topic：`/imu/data`、`/ultrasonic`、`/line_sensor`
- 订阅 topic：`/robot_cmd`（类型 `serial_comm/RobotControl`）

**`Robot_CTRL/` — 底盘控制节点**：

- 麦克纳姆轮运动学解算
- PID 控制（角速度闭环）
- 手柄遥控支持
- 三种控制模式：放松（RELAX）、停止（STOP）、正常（NORMAL）
- 订阅：`/motor_states`、`/imu/data`、`/joy`
- 发布：`/robot_cmd`

**其他包**: `robot_bringup/`（启动文件）、`third_packages/`（传感器驱动，同 v2）

### 10.5 MaixCam 视觉模块

- **运行环境**: MaixPy v4，代码通过 MaixVision 在 Windows 上编写，实际在 MaixCam 设备上运行
- **分辨率**: 优先使用 `320x240`，避免快帧缓冲内存不足

**当前脚本清单**（`MaixCam/maixcam_tests/`）:

| 脚本 | 功能 |
|------|------|
| `00_uart.py` | UART 基础收发测试（`/dev/ttyGS0` 或 `/dev/ttyS0`，115200） |
| `01_qrcode_test.py` | 二维码识别，打印内容并画框 |
| `02_color_blob_test.py` | 红绿蓝色块识别基础版 |
| `02_color_blob_center_test.py` | 色块中心点输出版（cx/cy/dx/dy） |
| `03_line_tracking_test.py` | 直线/巡线测试 |
| `04_vision_mode_test.py` | 综合模式测试（修改变量切换 QR/BLOB_RED/GREEN/BLUE/ALL/LINE） |
| `05_color_ring_test.py` | 基础色环中心识别（颜色阈值粗定位） |
| `06_color_ring_multi_circle_test.py` | 多真实圆检测与圆心融合实验版 |

**输出协议格式**:

```text
QR,123+231
BLOB,RED,dx,dy,cx,cy,w,h,area
RING,RED,dx,dy,cx,cy,radius,score,density,ratio,source
LINE,dx,theta
NONE,QR
NONE,BLOB,RED
```

### 10.6 MaixCam 硬件串口引脚

| UART | TX Pin | RX Pin | Linux 设备 | 说明 |
|------|--------|--------|-----------|------|
| **UART0** | A16 | A17 | `/dev/ttyS0` | 默认可用，开机输出 boot log；**A16 不能拉低否则无法开机** |
| **UART1** | A19 | A18 | `/dev/ttyS1` | 需 `pinmap.set_pin_function()` 配置后才可用 |
| UART2 | A28 | — | `/dev/ttyS2` | 与 JTAG_TDI 复用 |

- **IO 电压 3.3V**，不能直连 5V
- 接线：TX ↔ RX 交叉，GND ↔ GND
- UART0 启动时会输出系统日志，与 MCU 通信时需要忽略开机阶段的数据
- 如果 UART0 有问题，建议改用 UART1

### 10.7 系统架构规划

目标硬件拓扑：

```text
MaixCam --UART--> Jetson Nano / ROS --UART--> STM32F407 --电机/舵机/传感器
```

职责划分：

- **MaixCam**: 前端视觉传感器，仅输出识别结果（QR/BLOB/RING/LINE）
- **Jetson Nano**: 任务决策、状态机、路径规划、ROS 节点调度、数据融合
- **STM32**: 底层实时控制（电机闭环、舵机动作、机械爪时序、编码器、安全保护）

### 10.8 xrobot4_ws — Jetson Nano 上位机工作空间

存放我们的 Jetson Nano 端 ROS 代码，当前包含 MaixCam UART 串口桥接包。

#### 10.8.1 目录结构

```text
xrobot4_ws/
├── src/
│   ├── CMakeLists.txt                              # catkin 工作空间顶层
│   └── maixcam_uart_bridge/                        # MaixCam UART 桥接 ROS 包
│       ├── CMakeLists.txt
│       ├── package.xml
│       ├── setup.py
│       ├── launch/
│       │   └── maixcam_uart_bridge.launch          # 启动文件
│       ├── scripts/
│       │   └── maixcam_uart_bridge_node.py         # ROS 桥接节点
│       ├── src/maixcam_uart_bridge/
│       │   ├── __init__.py
│       │   └── protocol.py                         # 协议解析库
│       └── test/
│           └── test_protocol.py                    # 协议单元测试
```

#### 10.8.2 硬件接线

MaixCam ↔ Jetson Nano（UART 串口）：

```text
MaixCam A19 (TX)  ↔  Jetson RX  (USB-UART 转接器或 J41 pin 10)
MaixCam A18 (RX)  ↔  Jetson TX  (USB-UART 转接器或 J41 pin 8)
MaixCam GND       ↔  Jetson GND
```

推荐使用 USB-UART 转接器（CH340/FT232），插入 Jetson Nano USB 口后设备为 `/dev/ttyUSB0`。

#### 10.8.3 Jetson Nano 端部署

```bash
# 1. 将 xrobot4_ws 放到 Jetson Nano
cd ~/xrobot4_ws

# 2. 安装依赖
sudo apt install python3-pip
pip3 install pyserial

# 3. 编译
catkin_make
source devel/setup.bash

# 4. 启动 roscore + 桥接节点
roscore &
roslaunch maixcam_uart_bridge maixcam_uart_bridge.launch port:=/dev/ttyUSB0
```

#### 10.8.4 MaixCam 端运行

在 MaixVision 中打开并运行主脚本：

```text
MaixCam/maixcam_tests/maixcam_uart_vision.py
```

启动后 MaixCam 屏幕显示当前 MODE，终端打印 `HELLO,MAIXCAM_UART,115200`。

#### 10.8.5 切换视觉模式

在 Jetson 上通过 ROS topic 下发指令：

```bash
# 基础模式
rostopic pub /vision/mode std_msgs/String "data: 'MODE,QR'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,RED'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,GREEN'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,BLUE'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,ALL'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,RED'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,GREEN'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,BLUE'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,ALL'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,LINE'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,IDLE'" -1
```

也可以省略 `MODE,` 前缀（如 `QR`），节点会自动补全。

#### 10.8.6 查看 ROS 输出

```bash
rostopic echo /vision/status          # 连接状态
rostopic echo /vision/raw             # MaixCam 原始文本行
rostopic echo /vision/qrcode          # 二维码内容
rostopic echo /vision/blob/center     # 色块中心偏差 (x=dx, y=dy, z=area)
rostopic echo /vision/ring/center     # 色环中心偏差 (x=dx, y=dy, z=radius)
rostopic echo /vision/line            # 巡线偏差 (x=dx, theta=theta)
rostopic echo /vision/none            # 未识别到目标
```

#### 10.8.7 通信协议

MaixCam → Jetson（每行以 `\n` 结尾）：

```text
QR,123+231
BLOB,RED,dx,dy,cx,cy,w,h,area
RING,RED,dx,dy,cx,cy,radius,score,density,ratio,source
LINE,dx,theta
NONE,QR
NONE,BLOB,RED
NONE,RING,BLUE
INFO,MODE,BLOB_RED
HELLO,MAIXCAM_UART,115200
```

Jetson → MaixCam（每行以 `\n` 结尾）：

```text
MODE,QR
MODE,BLOB,RED
MODE,RING,RED
MODE,LINE
MODE,IDLE
PING
```

**坐标说明**：`cx, cy` 是目标中心点在图像中的绝对像素坐标，原点为图像左上角；`dx = cx - WIDTH//2`，`dy = cy - HEIGHT//2` 是相对画面中心的默认偏差。几何标定和坐标变换使用 `cx, cy`；闭环抓取时应优先使用目标相对抓取参考点 `(GRASP_U, GRASP_V)` 的误差，而不是默认假设目标必须回到画面中心。

#### 10.8.8 运行协议测试

```bash
cd ~/xrobot4_ws
python3 -m unittest discover -s src/maixcam_uart_bridge/test -v
```

#### 10.8.9 可复用的现有代码

- **xrobot3_ws 的 `serial_comm` 包**：Jetson ↔ STM32 串口通信，含 SD/ZD 二进制协议、CRC16 校验、IMU/超声波/巡线传感器数据发布。可直接复用或参照。
- **xrobot3_ws 的 `Robot_CTRL` 包**：麦克纳姆轮运动学解算、PID 控制，可在此基础上加入视觉闭环或自动导航逻辑。

### 10.9 后续实施步骤

1. ✅ 调通 MaixCam → Jetson Nano 的 **UART 串口** 通信（maixcam_uart_bridge）
2. Jetson 上运行 `maixcam_uart_bridge_node.py`，确认 ROS topic 有数据
3. 调通 Jetson Nano → STM32 的串口通信（可复用 xrobot3_ws 的 `serial_comm` 包）
4. Jetson 上写 `task_manager_node`，把视觉结果和 STM32 动作串成完整状态机
5. 最后做全流程联调

## 11. 眼在手上标定与抓取策略

当前机械结构按“相机跟随机械臂末端运动”理解，属于 **眼在手上（eye-in-hand）**。初赛评分要求物料精准摆放到色环内，且按环数计分，因此不能只依赖粗略的颜色识别，需要建立“图像坐标 -> 机械臂动作”的标定关系，并在实际抓取时保留视觉闭环微调。

### 11.1 内参、外参与手眼关系

**内参**描述 MaixCam 自身成像模型：

```text
fx, fy   焦距，单位为像素
cx, cy   主点，接近图像中心
dist     畸变参数，例如 k1, k2, p1, p2, k3
```

内参用于把图像上的标定板角点恢复成相机坐标系下的位姿。求外参/手眼关系前，应先完成内参标定。

**眼在手上外参**建议记录为：

```text
T_gripper_camera
```

含义是相机坐标系相对于机械臂末端/夹爪坐标系的固定变换。它用于回答：相机看到目标在某个位置时，机械臂末端应移动到哪里才能抓取。

### 11.2 数据采集顺序

采集时可以“拍照和记录末端状态同时做”，但计算时有明确顺序：

```text
第 i 次采集：
1. 机械臂运动到一个标定姿态并停稳
2. MaixCam 拍摄标定板图片 image_i
3. 记录此时末端位姿 T_base_gripper_i
   或记录关节角 q_i，后续通过正运动学计算末端位姿
```

后处理顺序：

```text
图片集 -> OpenCV calibrateCamera() -> camera_matrix, dist_coeffs
每张图片 + 内参 -> OpenCV solvePnP() -> T_camera_target_i
每组关节角/末端位姿 -> T_base_gripper_i
所有配对数据 -> OpenCV calibrateHandEye() -> T_gripper_camera
```

注意：OpenCV 的手眼标定不能直接只输入“照片 + 舵机角度”。它需要：

```text
R_gripper2base, t_gripper2base
R_target2cam,   t_target2cam
```

其中 `R_target2cam, t_target2cam` 由内参和 `solvePnP()` 得到；`R_gripper2base, t_gripper2base` 由机械臂末端位姿或关节角正运动学得到。

### 11.3 内参获取步骤

1. 固定比赛使用分辨率，建议先统一使用 `320x240`。
2. 打印棋盘格、ArUco 或 ChArUco 标定板，棋盘格方格边长建议 `20mm` 或 `25mm`。
3. 用 MaixCam 从不同角度、不同位置、不同距离拍摄 20-40 张标定板图片。
4. 将图片导出到电脑或 Jetson。
5. 使用 OpenCV `calibrateCamera()` 求 `camera_matrix` 和 `dist_coeffs`。
6. 保存为 `camera_intrinsics.yaml`，后续所有定位计算使用同一分辨率和同一组内参。

### 11.4 现有代码中可用的机械臂信息

当前没有发现现成的 ROS 末端位姿反馈 topic。

已有能力：

- `xrobot3_ws/src/serial_comm/msg/RobotControl.msg`：可下发 3 个关节角、腕部角、夹爪角和底盘电机速度。
- `xrobot3_ws/src/serial_comm/src/serial_node.cpp`：将 `/robot_cmd` 转成 Jetson -> STM32 的 `ZD` 控制帧。
- `8KTM-ROS/User/Task/src/thread_sensor.c`：STM32 侧接收 `ZD` 控制帧，保存到 `g_rc_command`；同时通过 `SD` 帧上传 IMU、超声波、巡线等传感器数据。
- `8KTM-ROS/User/Function/src/SCARA.c`：包含机械臂正解、反解和舵机 PWM/角度互转函数。

关键函数：

```c
calculate_SCARA_forward_Transform()  // 关节角 -> 末端坐标
calculate_delta()                    // 末端坐标 -> 关节角
Servo_PWMToAngle()                   // 舵机 PWM -> 机械臂角度
Servo_AngleToPWM()                   // 机械臂角度 -> 舵机 PWM
```

当前限制：

- ROS 端目前主要是下发目标角度，不发布真实关节角反馈。
- `thread_sensor.c` 中 `Robot_arm_task()` 里实际执行 `arm_ctrl(&g_rc_command)` 的调用目前是注释状态，需要联调确认是否启用。
- 短期做标定时，可以在机械臂停稳后把“下发的目标角度”近似当作当前关节角；更高精度方案需要 STM32 读取舵机当前位置并回传。

后续建议新增：

```text
STM32 读取舵机当前位置/当前角度
-> 加入 SD 反馈帧或新增机械臂状态帧
-> serial_comm 解析
-> ROS 发布 /arm/joint_states 或 /arm/end_effector_pose
```

### 11.5 如果暂时拿不到完整末端位姿

如果短期内只能获得舵机目标值，不能稳定获得真实末端位姿，可以先做“平面抓取标定”：

```text
图像坐标 (u, v) <-> 机械臂抓取平面坐标 (X, Y)
```

用 OpenCV `findHomography()` 建立映射：

```text
H: image(u,v) -> arm_plane(X,Y)
```

适用范围：

- 相机观察姿态固定；
- 目标在同一高度平面；
- 原料区抓取、车载承载区、粗加工区、暂存区可能需要分别标定。

平面标定比完整手眼标定简单，适合先把比赛流程跑起来；完整手眼标定适合后续提高通用性和速度。

### 11.6 实际抓取推荐流程

实际运行建议使用“两段式”：先根据标定快速到目标附近，再用视觉微调。微调时不要默认目标一定要回到画面中心，而要回到“夹爪真正能抓到目标时，目标在图像中的参考像素点”。

```text
1. 机械臂到观察位
2. MaixCam 识别目标，输出 cx, cy, dx, dy
3. 用手眼标定或平面标定估算目标在机械臂/底座坐标系的位置
4. 机械臂快速移动到预抓取位
5. 停稳后重新拍照
6. 根据新的 cx, cy 与抓取参考点的误差小步微调 X/Y
7. 误差进入阈值后下探
8. 夹爪闭合，完成抓取
```

重要原则：

- 机械臂每运动一次，旧图像只适合计算旧时刻的目标位置。
- 若目标不动，旧图像计算出的目标底座坐标仍可作为快速接近的参考。
- 做微调时应使用“最新图像 + 当前机械臂位姿/当前目标角度”。
- `dx, dy` 只是目标相对画面中心的偏差，适合调试；真正抓取应定义 `GRASP_U, GRASP_V`。
- 若相机光轴、夹爪中心和抓取点完全对齐，可以令 `GRASP_U = WIDTH/2`、`GRASP_V = HEIGHT/2`，此时 `err_u, err_v` 等价于 `dx, dy`。
- 若相机装在夹爪旁边或存在安装偏移，正确抓取时目标可能应该落在画面中的某个固定偏移位置，例如 `(GRASP_U, GRASP_V) = (190, 104)`，此时不应强行让 `dx = 0, dy = 0`。
- 初期不要边运动边连续计算，先采用“停稳 -> 拍照 -> 计算 -> 移动 -> 再停稳”的离散闭环。

推荐阈值可从以下范围开始调：

```text
err_u = cx - GRASP_U
err_v = cy - GRASP_V
abs(err_u) < 5-10 px
abs(err_v) < 5-10 px
连续 3 帧满足阈值后再下探抓取
```

### 11.7 推荐落地顺序

1. 固定 MaixCam 到机械臂末端，安装后不要再移动相机。
2. 固定 MaixCam 分辨率，优先使用 `320x240`。
3. 完成红、绿、蓝物料和色环 LAB 阈值标定。
4. 拍摄标定板图片，求 MaixCam 内参。
5. 确认机械臂能否获得当前关节角或末端位姿。
6. 若能获得末端位姿，做标准 eye-in-hand 手眼标定。
7. 若暂时不能获得末端位姿，先做每个工位的平面 Homography 标定。
8. 实现“观察位快速接近 + 预抓取位视觉微调 + 下探抓取”。
9. 后续再补 STM32 -> ROS 的机械臂状态反馈，提高标定和控制精度。

### 11.8 OpenCV 标定函数的输入、输出和使用方式

#### 11.8.1 `calibrateCamera()`：求相机内参

`calibrateCamera()` 不是直接把图片传进去就能自动算出内参。它需要两类点：

```text
objectPoints：标定板角点在真实世界中的坐标，单位通常是 mm
imagePoints：同一批角点在图片中的像素坐标，单位是 pixel
```

以棋盘格为例，如果棋盘格每格边长是 `25mm`，内角点数量是 `9 x 6`，则某一张图对应的真实角点可以写成：

```text
(0, 0, 0), (25, 0, 0), (50, 0, 0), ...
(0, 25, 0), (25, 25, 0), ...
```

因为棋盘格是平面，所以所有点的 `Z=0`。这些点就是 `objectPoints`。

然后 OpenCV 用 `findChessboardCorners()` 或 ArUco/ChArUco 检测图片中的角点，得到：

```text
(u1, v1), (u2, v2), (u3, v3), ...
```

这些像素坐标就是 `imagePoints`。

输入大致是：

```python
ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(
    objectPoints,
    imagePoints,
    imageSize,
    None,
    None
)
```

输出含义：

```text
camera_matrix：相机内参矩阵 K
dist_coeffs：镜头畸变参数
rvecs/tvecs：每张标定图片中，标定板相对于相机的位姿
ret：重投影误差，越小越好
```

内参矩阵通常长这样：

```text
K = [ fx  0  cx
      0  fy  cy
      0   0   1 ]
```

`fx, fy, cx, cy, dist_coeffs` 后续会用于 `solvePnP()`、图像去畸变和像素坐标转空间射线。

#### 11.8.2 `solvePnP()`：求某一张图中标定板相对于相机的位姿

`solvePnP()` 做的是 Perspective-n-Point 问题：已知一组 3D 点和它们在图像中的 2D 像素点，再结合相机内参，求“这个 3D 物体在相机坐标系下的位置和姿态”。

对手眼标定来说，它用来求：

```text
T_camera_target
```

含义是：标定板 target 坐标系相对于相机 camera 坐标系的变换。

输入：

```text
objectPoints：标定板角点真实坐标，例如棋盘格上的 (X,Y,0)
imagePoints：同一角点在当前图片里的像素坐标 (u,v)
camera_matrix：calibrateCamera() 得到的内参矩阵
dist_coeffs：calibrateCamera() 得到的畸变参数
```

调用形式：

```python
ok, rvec, tvec = cv2.solvePnP(
    objectPoints,
    imagePoints,
    camera_matrix,
    dist_coeffs
)
```

输出：

```text
rvec：旋转向量，可用 Rodrigues() 转成旋转矩阵 R_camera_target
tvec：平移向量，即标定板原点在相机坐标系下的位置
```

也就是说，`solvePnP()` 解决的是“这一张照片里，标定板在相机前方哪里、转了多少角度”。它不是手眼外参，但它是手眼标定必须用的一半数据。

#### 11.8.3 `calibrateHandEye()`：求眼在手上的外参

`calibrateHandEye()` 才是求手眼关系的函数。眼在手上时，目标是求：

```text
T_gripper_camera
```

含义是：相机坐标系相对于机械臂末端/夹爪坐标系的固定变换。只要相机没有重新安装，这个变换就是固定的。

它需要多组同步数据。每一组包括：

```text
T_base_gripper_i：第 i 次拍照时，机械臂末端在机械臂基坐标系下的位姿
T_camera_target_i：第 i 张图里，标定板在相机坐标系下的位姿
```

其中：

- `T_base_gripper_i` 来自机械臂正运动学，或来自机械臂控制器反馈；
- `T_camera_target_i` 来自 `solvePnP()`；
- 标定板在采集过程中应固定不动；
- 机械臂需要从多个不同位置和不同角度观察标定板。

OpenCV 常用输入形式：

```python
R_cam2gripper, t_cam2gripper = cv2.calibrateHandEye(
    R_gripper2base,
    t_gripper2base,
    R_target2cam,
    t_target2cam,
    method=cv2.CALIB_HAND_EYE_TSAI
)
```

注意命名容易混乱，写代码时必须统一坐标方向。推荐在自己的文档和代码里明确保存：

```text
T_base_gripper
T_camera_target
T_gripper_camera
```

如果 OpenCV 返回的是 `R_cam2gripper, t_cam2gripper`，它表示相机到夹爪或夹爪到相机的方向要根据 OpenCV 当前接口说明和实际测试确认。做完后应通过实测点验证方向是否正确，不要只相信变量名。

#### 11.8.4 得到内参和外参后怎么用

实际抓取时，最常见的坐标链是：

```text
图像绝对像素点 (u, v)，例如目标中心 (cx, cy)
-> 相机坐标系下的目标点 P_camera
-> 机械臂末端坐标系下的目标点 P_gripper
-> 机械臂基坐标系下的目标点 P_base
-> 机械臂逆解得到关节角
-> 下发关节角抓取
```

需要的变换：

```text
K                 相机内参
dist_coeffs       畸变参数
T_gripper_camera  手眼外参
T_base_gripper    当前机械臂末端位姿
```

这里的 `(u, v)` 使用图像左上角为原点的绝对像素坐标。不能把 `dx, dy` 当作 OpenCV 投影模型里的像素点直接使用，因为内参矩阵 `K` 中的主点坐标也是以图像左上角为原点定义的。

若已知目标在相机坐标系下的三维坐标 `P_camera`，则：

```text
P_gripper = T_gripper_camera * P_camera
P_base    = T_base_gripper * P_gripper
```

等价写成一条链：

```text
P_base = T_base_gripper * T_gripper_camera * P_camera
```

这里的 `P_base` 就是机械臂基坐标系下的目标位置，后续送入机械臂逆运动学，例如 STM32 中的 `calculate_delta()`，得到关节角。

#### 11.8.5 像素点如何变成 `P_camera`

相机看到的是二维像素 `(u, v)`，它本身没有深度。因此仅靠一个像素点不能直接得到三维点，必须额外知道深度或平面约束。对物料中心来说，`(u, v)` 就是视觉脚本输出的 `cx, cy`。

比赛中可用的简化条件：

```text
物料在转盘/桌面/色环平面上
抓取平面的高度已知
```

这时可以把像素点反投影成一条相机射线，再求这条射线和目标平面的交点。

概念流程：

```text
1. 用 dist_coeffs 去畸变像素点
2. 用 K^-1 把像素点转成相机坐标系下的归一化射线
3. 根据已知抓取平面求射线与平面的交点
4. 得到 P_camera
5. 用 P_base = T_base_gripper * T_gripper_camera * P_camera 转到机械臂基坐标系
```

`dx, dy` 不参与这一步几何反投影；它只是 `cx, cy` 减去图像中心后的结果。若后续需要相对抓取点闭环，应单独计算：

```text
err_u = cx - GRASP_U
err_v = cy - GRASP_V
```

如果暂时不想处理完整三维射线和平面方程，可以先用每个工位单独的 Homography：

```text
image(u,v) -> arm_plane(X,Y)
```

这种方法本质上把“内参、外参、平面约束”合在一个二维映射里，简单实用，但只适用于同一观察姿态和同一平面。

#### 11.8.6 实际代码里建议保存的文件

建议后续生成并保存：

```text
calibration/
├── camera_intrinsics.yaml      # camera_matrix, dist_coeffs, image_size
├── hand_eye.yaml               # T_gripper_camera
├── workcell_homography.yaml    # 各工位的 H，可选
└── calibration_samples.csv     # 每张图对应的关节角/末端位姿记录
```

实际运行时：

```text
1. 读取 camera_intrinsics.yaml 和 hand_eye.yaml
2. 机械臂到观察位并记录/估算 T_base_gripper
3. MaixCam 输出目标像素中心 cx, cy
4. 通过平面约束或 Homography 计算目标坐标
5. 转成机械臂基坐标系下的 P_base
6. 调用逆运动学得到关节角
7. 下发预抓取动作
8. 停稳后重新拍照，用 cx, cy 相对抓取参考点 GRASP_U, GRASP_V 的误差做小范围微调
```
