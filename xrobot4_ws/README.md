# xrobot4_ws: Jetson Nano 与 MaixCam USB 通信

这个工作空间用于把 MaixCam 的视觉识别结果通过 USB 线传到 Jetson Nano 的 ROS1 系统中。

## 方案说明

MaixCam 的 Type-C USB 直连默认表现为 USB 虚拟网卡，不是普通 `/dev/ttyUSB*` 串口。Linux 上插入后通常会出现 `usb0` 或 `usb1`，Jetson 这边常见 IP 类似 `10.131.167.100`，MaixCam 设备端 IP 是同网段最后一位改成 `1`，通常为：

```text
10.131.167.1
```

因此本目录采用：

```text
MaixCam 视觉脚本 --USB 虚拟网卡/TCP:7000--> Jetson ROS maixcam_bridge
```

## 目录结构

```text
xrobot4_ws/
  README.md
  maixcam/
    usb_tcp_vision_server.py        # 在 MaixCam/MaixVision 上运行
  src/
    CMakeLists.txt
    maixcam_bridge/
      launch/maixcam_bridge.launch
      scripts/maixcam_bridge_node.py
      src/maixcam_bridge/protocol.py
      test/test_protocol.py
```

## 一、Jetson Nano 端准备

把整个 `xrobot4_ws` 放到 Jetson Nano，例如：

```bash
cd ~/xrobot4_ws
catkin_make
source devel/setup.bash
```

建议加入 `~/.bashrc`：

```bash
echo "source ~/xrobot4_ws/devel/setup.bash" >> ~/.bashrc
```

检查 USB 网卡：

```bash
ip addr | grep -A3 -E "usb0|usb1"
ping 10.131.167.1
```

如果看不到 `usb0/usb1`，先换一根支持数据传输的 Type-C 线，并确认 MaixCam 已开机进入系统。

## 二、MaixCam 端运行脚本

用 MaixVision 连接 MaixCam，打开并运行：

```text
xrobot4_ws/maixcam/usb_tcp_vision_server.py
```

运行后 MaixCam 屏幕会显示当前 `MODE`，终端会打印：

```text
MaixCam USB TCP vision server started on port 7000.
```

脚本默认监听：

```text
0.0.0.0:7000
```

Jetson 连接后会收到：

```text
HELLO,MAIXCAM_USB_TCP,7000
```

## 三、启动 ROS 桥接节点

在 Jetson 上运行：

```bash
roscore
```

另开终端：

```bash
cd ~/xrobot4_ws
source devel/setup.bash
roslaunch maixcam_bridge maixcam_bridge.launch
```

如果 MaixCam IP 不是默认值，可以指定：

```bash
roslaunch maixcam_bridge maixcam_bridge.launch host:=10.131.167.1 port:=7000
```

## 四、切换视觉模式

桥接节点订阅：

```text
/vision/mode
```

可发送以下模式：

```bash
rostopic pub /vision/mode std_msgs/String "data: 'MODE,QR'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,RED'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,GREEN'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,BLUE'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,BLOB,ALL'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,RED'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,RING,ALL'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,LINE'" -1
rostopic pub /vision/mode std_msgs/String "data: 'MODE,IDLE'" -1
```

也可以省略 `MODE,` 前缀，例如发送 `QR`，桥接节点会自动补成 `MODE,QR`。

## 五、查看 ROS 输出

常用 topic：

```text
/vision/status         连接状态、HELLO、INFO、WARN、ERR
/vision/raw            MaixCam 原始文本行
/vision/qrcode         二维码内容，例如 123+231
/vision/blob/raw       色块原始结果
/vision/blob/center    色块中心，x=cx, y=cy, z=area
/vision/ring/raw       色环原始结果
/vision/ring/center    色环中心，x=cx, y=cy, z=radius
/vision/line/raw       巡线原始结果
/vision/line           巡线结果，x=dx, theta=theta
/vision/none           未识别到目标
```

测试命令：

```bash
rostopic echo /vision/status
rostopic echo /vision/raw
rostopic echo /vision/qrcode
rostopic echo /vision/blob/center
```

## 六、通信协议

MaixCam 发给 Jetson：

```text
QR,123+231
BLOB,RED,cx,cy,w,h,area,dx,dy
RING,BLUE,cx,cy,dx,dy,radius,score,density,ratio,source
LINE,dx,theta
NONE,QR
NONE,BLOB,RED
NONE,RING,BLUE
ERR,UNKNOWN_MODE,xxx
INFO,MODE,BLOB_RED
```

Jetson 发给 MaixCam：

```text
MODE,QR
MODE,BLOB,RED
MODE,BLOB,GREEN
MODE,BLOB,BLUE
MODE,BLOB,ALL
MODE,RING,RED
MODE,RING,GREEN
MODE,RING,BLUE
MODE,RING,ALL
MODE,LINE
MODE,IDLE
PING
```

所有命令以换行 `\n` 结尾。

## 七、本地测试

在电脑或 Jetson 上可先跑协议解析测试：

```bash
cd ~/xrobot4_ws
python3 -m unittest discover -s src/maixcam_bridge/test -v
```

期望结果：

```text
Ran 7 tests
OK
```

## 八、常见问题

### 1. `ping 10.131.167.1` 不通

先确认 USB 线支持数据传输，不是只能充电。再检查：

```bash
ip addr
```

Linux 通常无需驱动，插上后会出现 `usb0` 或 `usb1`。

### 2. ROS 节点一直显示连接失败

确认 MaixCam 端脚本正在运行，并且端口是 `7000`。Jetson 上可测试：

```bash
nc -vz 10.131.167.1 7000
```

### 3. 能连接但没有识别结果

先发模式：

```bash
rostopic pub /vision/mode std_msgs/String "data: 'MODE,QR'" -1
```

然后看：

```bash
rostopic echo /vision/raw
```

### 4. 颜色识别不稳定

当前 LAB 阈值是起始值。实际比赛前要用 MaixCam 自带 `Find Blobs` 工具重新标定红、绿、蓝物料和色环阈值，然后改 `usb_tcp_vision_server.py` 里的 `COLOR_CONFIGS` 和 `RING_CONFIGS`。

## 参考资料

- Sipeed MaixPy Quick Start：https://en.wiki.sipeed.com/maixpy/doc/en/index.html
- Sipeed MaixPy FAQ：https://wiki.sipeed.com/maixpy/doc/en/faq.html
- Sipeed MaixPy 二维码识别：https://en.wiki.sipeed.com/maixpy/doc/zh/vision/qrcode.html
- Sipeed MaixPy 巡线识别：https://wiki.sipeed.com/maixpy/doc/en/vision/line_tracking.html
- Sipeed MaixPy UART 串口：https://wiki.sipeed.com/maixpy/doc/en/peripheral/uart.html
