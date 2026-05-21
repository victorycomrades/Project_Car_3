# MaixCam × Jetson Nano UART 串口通信协议说明

## 1. 硬件接线

```
MaixCam (UART1)          Jetson Nano
A19 (TX)           →     RX (USB-UART 转接器)
A18 (RX)           →     TX (USB-UART 转接器)
GND                →     GND
```

- IO 电压 **3.3V**，5V 设备需加电平转换
- USB-UART 转接器推荐 CH340 / FT232
- 插入 Jetson 后设备路径通常为 `/dev/ttyUSB0`

---

## 2. 串口参数

| 参数 | 值 |
|------|-----|
| 波特率 | 115200 |
| 数据位 | 8 |
| 校验位 | 无 |
| 停止位 | 1 |

MaixCam 端使用 UART1：设备节点 `/dev/ttyS1`（引脚 A18/A19），启动时自动配置 pinmap。

---

## 3. Jetson → MaixCam 指令集

每条指令以换行符 `\n` 结尾。

### 3.1 模式切换指令

| 指令 | 功能 |
|------|------|
| `MODE,QR` | 二维码识别 |
| `MODE,BLOB,RED` | 只找 **红色** 物料 |
| `MODE,BLOB,GREEN` | 只找 **绿色** 物料 |
| `MODE,BLOB,BLUE` | 只找 **蓝色** 物料 |
| `MODE,BLOB,ALL` | 同时找红绿蓝三种物料 |
| `MODE,RING,RED` | 只找 **红色** 色环（放置区定位） |
| `MODE,RING,GREEN` | 只找 **绿色** 色环 |
| `MODE,RING,BLUE` | 只找 **蓝色** 色环 |
| `MODE,RING,ALL` | 同时找红绿蓝三种色环 |
| `MODE,LINE` | 巡线模式（车道线检测） |
| `MODE,IDLE` | 空闲模式，不收发识别数据 |

### 3.2 心跳检测

| 指令 | 响应 |
|------|------|
| `PING` | `PONG,MAIXCAM` |

---

## 4. MaixCam → Jetson 输出格式

每行以 `\n` 结尾，字段逗号分隔。

### 4.1 二维码

```
QR,payload,dx,dy
```

| 字段 | 说明 |
|------|------|
| payload | 二维码内容，如 `123+231` |
| dx | 二维码中心相对画面中心的横向偏差 (px) |
| dy | 二维码中心相对画面中心的纵向偏差 (px) |

未识别到时：
```
NONE,QR
```

### 4.2 物料色块

```
BLOB,color,dx,dy,cx,cy,w,h,area
```

| 字段 | 说明 |
|------|------|
| color | 颜色：`RED` / `GREEN` / `BLUE` |
| dx | 目标中心 X 偏差 `= cx - 160` |
| dy | 目标中心 Y 偏差 `= cy - 120` |
| cx | 目标中心在画面中的绝对 X 坐标 |
| cy | 目标中心在画面中的绝对 Y 坐标 |
| w | 包围框宽度 (px) |
| h | 包围框高度 (px) |
| area | 包围框面积 `= w × h` |

未识别到时：
```
NONE,BLOB,RED
```

### 4.3 色环（放置区定位）

```
RING,color,dx,dy,cx,cy,radius,score,density,ratio,source
```

| 字段 | 说明 |
|------|------|
| color | 颜色：`RED` / `GREEN` / `BLUE` |
| dx | 中心 X 偏差 |
| dy | 中心 Y 偏差 |
| cx | 绝对 X 坐标 |
| cy | 绝对 Y 坐标 |
| radius | 检测到的半径 (px) |
| score | 置信度得分 |
| density | 像素填充密度百分比 |
| ratio | 方形比例 `min(w,h)/max(w,h)×100` |
| source | 检测来源：`BLOB` / `SINGLE_HOUGH1` / `MULTI_HOUGHn` |

未识别到时：
```
NONE,RING,RED
```

### 4.4 巡线

```
LINE,dx,theta
```

| 字段 | 说明 |
|------|------|
| dx | 车道线相对画面中心的横向偏差 (px) |
| theta | 车道线角度（-90 ~ 90） |

未检测到线时：
```
NONE,LINE
```

### 4.5 状态消息

| 格式 | 说明 |
|------|------|
| `HELLO,MAIXCAM_UART,115200` | 启动握手 |
| `INFO,MODE,BLOB_RED` | 模式切换确认 |
| `INFO,IDLE` | 空闲心跳（每 60 帧） |
| `ERR,UNKNOWN_MODE,xxx` | 未知模式错误 |
| `ERR,UNKNOWN_COMMAND,xxx` | 未知指令错误 |
| `PONG,MAIXCAM` | Ping 响应 |

---

## 5. Jetson 端发送方式

### 5.1 通过 ROS topic（推荐）

```bash
# 启动 bridge 节点后，用 rostopic 发送
rostopic pub /vision/mode std_msgs/String "data: 'MODE,QR'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,RED'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,RED'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,LINE'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,IDLE'" -1
```

可省略 `MODE,` 前缀，bridge 节点自动补全：
```bash
rostopic pub /vision/mode std_msgs/String "data: 'QR'" -1
```

### 5.2 直接串口发送（测试用）

```bash
echo "MODE,QR" > /dev/ttyUSB0
echo "MODE,BLOB,RED" > /dev/ttyUSB0
echo "PING" > /dev/ttyUSB0
```

### 5.3 Python 代码发送

```python
import serial

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.02)

# 发送模式切换
ser.write(b"MODE,BLOB,RED\n")

# 发心跳
ser.write(b"PING\n")

# 读取 MaixCam 返回的数据
while True:
    line = ser.readline()
    if line:
        print(line.decode().strip())
```

---

## 6. 坐标体系说明

画面分辨率 `320×240`，坐标系：

```
        dx < 0          dx > 0
          ←───[目标]───→
                │
    (0,0) ──────●────── (320,0)     ← 画面左上角
                │ (160,120) 画面中心
                │
        dy < 0  │  dy > 0
                ↓
```

- **dx > 0**：目标在画面中心右侧 → 机械臂向右移动
- **dx < 0**：目标在画面中心左侧 → 机械臂向左移动
- **dy > 0**：目标在画面中心下方 → 机械臂向下移动
- **dy < 0**：目标在画面中心上方 → 机械臂向上移动

机械臂闭环控制直接用 `(dx, dy)` 做位置偏差输入。

---

## 7. 查看 MaixCam 输出（调试）

### 7.1 MaixVision 终端

在 MaixVision 中运行 `maixcam_uart_vision.py`，终端窗口中 `print()` 输出的内容即为发送到串口的原始数据。

### 7.2 Jetson ROS

```bash
# 查看原始串口数据
rostopic echo /vision/raw

# 查看二维码
rostopic echo /vision/qrcode

# 查看色块中心偏差（PointStamped: x=dx, y=dy, z=area）
rostopic echo /vision/blob/center

# 查看色环中心偏差（PointStamped: x=dx, y=dy, z=radius）
rostopic echo /vision/ring/center

# 查看巡线偏差（Pose2D: x=dx, theta=theta）
rostopic echo /vision/line
```

### 7.3 直接串口监听

```bash
# 在 Jetson 上用 cat 看串口原始数据
cat /dev/ttyUSB0
```

---

## 8. 典型工作流程

```text
1. MaixCam 启动 → 发 HELLO → 默认 IDLE 模式
2. Jetson 发 MODE,QR → MaixCam 切到二维码识别 → 返回 INFO,MODE,QR
3. MaixCam 持续发送 QR,123+231,dx,dy（拍到二维码后）
4. Jetson 收到任务码 → 发 MODE,IDLE → 停止二维码识别
5. Jetson 解析搬运顺序 → 发 MODE,BLOB,RED → 开始找红色物料
6. MaixCam 持续发送 BLOB,RED,dx,dy,...
7. Jetson 根据 dx,dy 控制机械臂对准 → 抓取 → 发 MODE,BLOB,GREEN
8. ... 依次完成全部物料抓取、放置
```
