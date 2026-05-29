# 修改日志

## 2026-05-19 (第二部分) — xrobot2_ws 串口节点 rosserial → SD/ZD 协议替换

### 问题

`xrobot_bringup.launch` 启动时报错：
```
[ERROR] Unable to sync with device; possible link problem or link software version
mismatch such as hydro rosserial_python with groovy Arduino
```

### 根因

xrobot2_ws 的 `serial_node.py` 使用 **rosserial_python** 与 STM32 通信。rosserial 要求 STM32 固件也集成 rosserial 库（类似 Arduino `ros.h`），但 8KTM-ROS 里的 STM32 固件是裸 HAL 代码，不含 rosserial 协议支持。rosserial 握手失败 → 持续报 ERROR。

而 xrobot3_ws 的 `serial_comm` 使用 **LibSerial + 自定义 SD/ZD 二进制协议**，不依赖 rosserial，所以能正常启动。

### 修改内容

#### 1. 新建 `serial_node_new.py`
`xrobot2_ws/src/xrobot_driver/scripts/serial_node_new.py`

- 用 **pyserial** 替代 rosserial_python，SD/ZD 二进制协议与 xrobot3_ws `serial_comm` 完全兼容
- 实现了完整的 CRC16-IBM 校验、SD 帧解析、ZD 帧构建
- STM32 未连接时：WARN 级别日志 + 自动重试，不阻塞 bringup 其他节点启动
- 发布 topic：`/imu/data`、`/ultrasonic`、`/line_sensor`（与 xrobot3_ws 一致）
- 订阅 topic：`/robot_cmd`（std_msgs/String，10 个逗号分隔整数值：m1,m2,m3,m4,j1,j2,j3,wrist,gripper,reset）

#### 2. 修改 `xrobot_core.launch`
`xrobot2_ws/src/xrobot_driver/launch/xrobot_core.launch`

- 节点从 `serial_node.py`（rosserial）改为 `serial_node_new.py`（pyserial + SD/ZD）

#### 3. 修改 `xrobot_bringup.launch`
`xrobot2_ws/src/xrobot_driver/launch/xrobot_bringup.launch`

- 波特率从 `512000` 改为 `115200`（匹配 STM32 固件 USART2 和 xrobot3_ws）

### 使用方式

```bash
cd ~/xrobot2_ws
catkin_make
source devel/setup.bash
roslaunch xrobot_driver xrobot_bringup.launch
```

现在 STM32 即使未连接也不会报 ERROR，节点会静默重试直到串口可用。

---

## 2026-05-19 (第一部分) — MaixCam UART 串口通信 + 坐标系改造

### 修改目标

1. 将 MaixCam 与 Jetson Nano 的通信方式从打印输出改为 **UART 串口双向通信**
2. 将视觉识别结果的坐标输出改为 **(dx, dy) = 相对于相机画面中心的偏差**，方便机械臂直接使用
3. 清空旧的 xrobot4_ws（TCP/USB 虚拟网卡方案），新建基于 UART 的 ROS 桥接包

---

### 一、MaixCam 端修改（MaixCam/maixcam_tests/）

#### 1.1 新增文件

**`maixcam_uart_vision.py`** —— MaixCam UART 双目通信主生产脚本

- 通过 **UART1**（`/dev/ttyS1`，引脚 A18/A19）与 Jetson 通信，115200 波特率
- 启动时配置 pinmap 将 A18/A19 设为 UART1 功能
- 接收 Jetson 发来的 `MODE,xxx` 指令，实时切换视觉模式
- 支持全部模式：`QR`、`BLOB_RED/GREEN/BLUE/ALL`、`RING_RED/GREEN/BLUE/ALL`、`LINE`、`IDLE`
- 所有识别结果通过 UART 发送（含 dx, dy 中心偏差）
- 色环检测包含多圆霍夫融合（`USE_MULTI_CIRCLE = True`）
- 屏幕上显示当前 MODE 状态和识别结果

#### 1.2 修改文件

所有测试脚本均新增以下功能：

| 脚本 | 修改内容 |
|------|----------|
| `01_qrcode_test.py` | 新增 UART 输出（可选开关）；输出格式 `QR,payload` |
| `02_color_blob_test.py` | 新增 UART 输出（可选开关）；新增 dx,dy 计算和输出；输出格式 `BLOB,color,dx,dy,cx,cy,w,h` |
| `02_color_blob_center_test.py` | 新增 UART 输出（可选开关）；dx,dy 已在之前版本存在，输出格式标准化 |
| `03_line_tracking_test.py` | 新增 UART 输出（可选开关）；将 rho 转为 dx（`dx = rho - WIDTH//2`），输出 `LINE,dx,theta` |
| `04_vision_mode_test.py` | 新增 UART 双向通信：**可接收 MODE 指令切换模式**，输出识别结果；输出格式标准化为 dx,dy 前置 |
| `05_color_ring_test.py` | 新增 UART 输出（可选开关）；RING 输出格式调整为 dx,dy 前置 |
| `06_color_ring_multi_circle_test.py` | 新增 UART 输出（可选开关）；RING 输出格式调整为 dx,dy 前置 |

**每个测试脚本的统一模式**：
- 顶部增加 `UART_DEVICE = None` 配置项（设为 `None` 则保持纯 print 模式，与修改前行为一致）
- 增加 `uart_init()` / `uart_write()` 辅助函数
- 所有坐标输出格式统一为 **dx, dy 前置**（`BLOB,color,dx,dy,cx,cy,w,h,area` 和 `RING,color,dx,dy,cx,cy,radius,...`）

---

### 二、Jetson Nano 端修改（xrobot4_ws/）

#### 2.1 删除的旧文件

```
xrobot4_ws/maixcam/usb_tcp_vision_server.py   ← TCP 方案（已删除）
xrobot4_ws/src/maixcam_bridge/                 ← TCP ROS 包（已删除）
xrobot4_ws/README.md                           ← TCP 方案说明（已删除）
```

#### 2.2 新建的 ROS 包：`maixcam_uart_bridge`

```
xrobot4_ws/src/maixcam_uart_bridge/
├── CMakeLists.txt                              # catkin 构建配置
├── package.xml                                 # 包元数据（依赖 rospy, std_msgs, geometry_msgs）
├── setup.py                                    # Python 包安装
├── launch/
│   └── maixcam_uart_bridge.launch              # 启动文件（可配置 port/baud/topic_prefix）
├── scripts/
│   └── maixcam_uart_bridge_node.py             # ROS 主节点
├── src/maixcam_uart_bridge/
│   ├── __init__.py
│   └── protocol.py                             # 文本协议解析库
└── test/
    └── test_protocol.py                        # 单元测试（10 个用例）
```

**maixcam_uart_bridge_node.py 核心逻辑**：

- 使用 **pyserial** 打开串口（默认 `/dev/ttyUSB0`，115200）
- 非阻塞读取（50Hz）→ 缓冲 → 解析完整行 → 分发到 ROS topic
- 订阅 `/vision/mode` 话题，收到指令后序列化并通过串口发送给 MaixCam
- 自动重连：断线后等待 1 秒重新打开串口
- 发布的状态 topic 含 `latch=True`，新订阅者可获取最后状态

**protocol.py 解析格式**（与 MaixCam 端输出格式一致）：

```
QR,payload
BLOB,color,dx,dy,cx,cy,w,h,area
RING,color,dx,dy,cx,cy,radius,score,density,ratio,source
LINE,dx,theta
NONE,type[,color]
HELLO / INFO / WARN / ERR / PONG
```

**ROS Topic 一览**：

| Topic | 类型 | 内容 |
|-------|------|------|
| `/vision/raw` | String | MaixCam 原始文本行 |
| `/vision/status` | String (latched) | 连接状态 |
| `/vision/qrcode` | String | 二维码内容 (payload) |
| `/vision/blob/raw` | String | 色块原始文本 |
| `/vision/blob/center` | PointStamped | x=dx, y=dy, z=area |
| `/vision/ring/raw` | String | 色环原始文本 |
| `/vision/ring/center` | PointStamped | x=dx, y=dy, z=radius |
| `/vision/line/raw` | String | 巡线原始文本 |
| `/vision/line` | Pose2D | x=dx, theta=theta |
| `/vision/none` | String | 未识别目标 |
| `/vision/mode` (sub) | String | 接收模式切换指令 |

---

### 三、坐标体系变更

**之前**：输出 `(cx, cy)`，坐标系原点在图像左上角 (0, 0)
**现在**：输出 `(dx, dy)` 前置，坐标系原点在图像中心 (160, 120)

```text
dx = cx - 160      # 正值 = 目标在画面中心右侧
dy = cy - 120      # 正值 = 目标在画面中心下方
```

机械臂控制时直接用 `(dx, dy)` 做闭环对准：若目标偏右（dx > 0），机械臂向右移动；若目标偏下（dy > 0），机械臂向下移动。

---

### 四、README.md 更新

- 新增 **第 10 节**：项目目录结构与硬件通信梳理（STM32 串口表、ROS 包结构、MaixCam 引脚表、架构规划）
- 新增 **10.8 节**：xrobot4_ws 详细文档（目录结构、硬件接线、部署步骤、通信协议、ROS topic、测试方法）

---

### 五、使用指南

#### 硬件接线

```
MaixCam (UART1)         Jetson Nano
A19 (TX)          →     RX (USB-UART)
A18 (RX)          →     TX (USB-UART)
GND               →     GND
```

#### MaixCam 端（MaixVision）

运行 `maixcam_uart_vision.py`（生产模式）或任意测试脚本（将 `UART_DEVICE = None` 改为 `"/dev/ttyS1"`）。

#### Jetson 端

```bash
cd ~/xrobot4_ws
catkin_make
source devel/setup.bash
roslaunch maixcam_uart_bridge maixcam_uart_bridge.launch port:=/dev/ttyUSB0
```

#### 测试

```bash
# 切换到二维码识别
rostopic pub /vision/mode std_msgs/String "data: 'MODE,QR'" -1

# 查看结果
rostopic echo /vision/qrcode
```

---

### 六、已知注意事项

1. **UART0 vs UART1**：UART0（`/dev/ttyS0`）开机输出系统日志，推荐使用 UART1（`/dev/ttyS1`）避免干扰
2. **A16 引脚**：如果使用 UART0，A16 (TX) 在开机时拉低会导致 MaixCam 无法启动
3. **电平**：MaixCam IO 电压为 3.3V，连接 5V 设备需电平转换
4. **pyserial**：Jetson Nano 需要 `pip3 install pyserial`
5. **测试脚本 UART 开关**：测试脚本默认 `UART_DEVICE = None`（仅打印），需要串口输出时改为 `"/dev/ttyS0"` 或 `"/dev/ttyS1"`
6. **MODE 命令格式**：Jetson 发 `MODE,BLOB,RED`，MaixCam 内部转成 `BLOB_RED`。也可以发缩写 `QR` 自动补全
