#!/usr/bin/env python3
"""
MaixCam UART Bridge — ROS1 节点
================================

功能：
  - 通过串口接收 MaixCam 发来的视觉识别结果（QR/BLOB/RING/LINE）
  - 解析文本协议并发布到 ROS topic
  - 订阅 /vision/mode 话题，将模式指令转发给 MaixCam
  - 自动重连

启动方式：
  roslaunch maixcam_uart_bridge maixcam_uart_bridge.launch

  或指定端口：
  roslaunch maixcam_uart_bridge maixcam_uart_bridge.launch port:=/dev/ttyUSB0

ROS topic 说明：
  /vision/raw           原始文本行
  /vision/status        连接状态 / HELLO / INFO / WARN / ERR
  /vision/qrcode        二维码内容
  /vision/blob/raw      色块原始结果
  /vision/blob/center   色块中心偏差 (x=dx, y=dy, z=area)
  /vision/ring/raw      色环原始结果
  /vision/ring/center   色环中心偏差 (x=dx, y=dy, z=radius)
  /vision/line/raw      巡线原始结果
  /vision/line          巡线偏差 (x=dx, theta=theta)
  /vision/none          未识别到目标
"""

import collections
import os
import signal
import sys
import threading
import time

import serial
import serial.tools.list_ports

import rospy
from geometry_msgs.msg import PointStamped, Pose2D
from std_msgs.msg import String

# 将 src/ 加入路径以导入 protocol
_pkg_src = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "src"))
if _pkg_src not in sys.path:
    sys.path.insert(0, _pkg_src)

from maixcam_uart_bridge.protocol import format_mode_command, parse_line  # noqa: E402


class MaixCamUartBridgeNode:
    def __init__(self):
        self.port = rospy.get_param("~port", "/dev/ttyUSB0")
        self.baud = int(rospy.get_param("~baud", 115200))
        self.topic_prefix = rospy.get_param("~topic_prefix", "/vision").rstrip("/")
        self.frame_id = rospy.get_param("~frame_id", "maixcam")
        self.reconnect_delay = float(rospy.get_param("~reconnect_delay", 1.0))

        self.ser = None
        self.rx_buffer = ""
        self._cmd_lock = threading.Lock()
        self._cmd_queue = collections.deque()
        self._shutdown = False

        # Publishers
        self.pub_raw = rospy.Publisher(self.topic_prefix + "/raw", String, queue_size=20)
        self.pub_status = rospy.Publisher(self.topic_prefix + "/status", String,
                                          queue_size=5, latch=True)
        self.pub_qrcode = rospy.Publisher(self.topic_prefix + "/qrcode", String, queue_size=5)
        self.pub_blob_raw = rospy.Publisher(self.topic_prefix + "/blob/raw", String, queue_size=10)
        self.pub_blob_center = rospy.Publisher(self.topic_prefix + "/blob/center",
                                               PointStamped, queue_size=10)
        self.pub_ring_raw = rospy.Publisher(self.topic_prefix + "/ring/raw", String, queue_size=10)
        self.pub_ring_center = rospy.Publisher(self.topic_prefix + "/ring/center",
                                               PointStamped, queue_size=10)
        self.pub_line_raw = rospy.Publisher(self.topic_prefix + "/line/raw", String, queue_size=10)
        self.pub_line = rospy.Publisher(self.topic_prefix + "/line", Pose2D, queue_size=10)
        self.pub_none = rospy.Publisher(self.topic_prefix + "/none", String, queue_size=10)

        # Subscriber — 接收模式切换指令
        rospy.Subscriber(self.topic_prefix + "/mode", String,
                         self._mode_callback, queue_size=10)

        rospy.loginfo("MaixCam UART Bridge initialized. Port: %s, Baud: %d",
                      self.port, self.baud)

    # ---- 生命周期 ----------------------------------------------------------

    def _mode_callback(self, msg):
        command = msg.data.strip()
        if not command:
            return
        with self._cmd_lock:
            self._cmd_queue.append(command)

    def _send_pending_commands(self):
        """发送队列中等待的指令到 MaixCam。"""
        with self._cmd_lock:
            while self._cmd_queue:
                command = self._cmd_queue.popleft()
                try:
                    payload = format_mode_command(command)
                    self.ser.write(payload)
                    rospy.loginfo("Sent to MaixCam: %s", command)
                except Exception as exc:
                    rospy.logwarn("Failed to send command %r: %s", command, exc)
                    self._cmd_queue.appendleft(command)
                    break

    def _read_available_lines(self):
        """从串口读取数据，处理完整行。"""
        try:
            waiting = self.ser.in_waiting
        except (OSError, serial.SerialException):
            raise ConnectionError("Serial port disconnected")

        if waiting > 0:
            try:
                chunk = self.ser.read(min(waiting, 1024))
            except (OSError, serial.SerialException) as exc:
                raise ConnectionError("Serial read error: %s" % exc)

            if not chunk:
                raise ConnectionError("Serial port returned empty read")
            self.rx_buffer += chunk.decode("utf-8", errors="replace")

        while "\n" in self.rx_buffer:
            line, self.rx_buffer = self.rx_buffer.split("\n", 1)
            line = line.strip()
            if line:
                self._publish_parsed(line)

    def _publish_parsed(self, line):
        """解析一行协议文本并发布到对应 topic。"""
        self.pub_raw.publish(String(data=line))

        try:
            msg = parse_line(line)
        except ValueError as exc:
            rospy.logdebug("Skipping malformed MaixCam line %r: %s", line, exc)
            return

        if msg.kind == "QR":
            self.pub_qrcode.publish(String(data=msg.payload or ""))

        elif msg.kind == "BLOB":
            self.pub_blob_raw.publish(String(data=msg.raw))
            self._publish_point(self.pub_blob_center, msg, "area")

        elif msg.kind == "RING":
            self.pub_ring_raw.publish(String(data=msg.raw))
            self._publish_point(self.pub_ring_center, msg, "radius")

        elif msg.kind == "LINE":
            pose = Pose2D()
            pose.x = float(msg.fields.get("dx", 0))
            pose.y = 0.0
            pose.theta = float(msg.fields.get("theta", 0))
            self.pub_line_raw.publish(String(data=msg.raw))
            self.pub_line.publish(pose)

        elif msg.kind == "NONE":
            self.pub_none.publish(String(data=msg.raw))

        elif msg.kind in ("HELLO", "INFO", "WARN", "ERR", "PONG"):
            self.pub_status.publish(String(data=msg.raw))

    def _publish_point(self, publisher, msg, z_field):
        """发 PointStamped，x=dx, y=dy, z=面积/半径等辅助字段。"""
        point = PointStamped()
        point.header.stamp = rospy.Time.now()
        point.header.frame_id = self.frame_id
        point.point.x = float(msg.fields.get("dx", 0))
        point.point.y = float(msg.fields.get("dy", 0))
        point.point.z = float(msg.fields.get(z_field, 0)) if z_field else 0.0
        publisher.publish(point)

    # ---- 连接管理 ----------------------------------------------------------

    def _connect(self):
        self._disconnect()
        rospy.loginfo("Opening serial port %s @ %d baud ...", self.port, self.baud)
        self.ser = serial.Serial(
            port=self.port,
            baudrate=self.baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.02,       # 非阻塞读取
        )
        self.rx_buffer = ""
        self.pub_status.publish(String(data="CONNECTED,%s,%d" % (self.port, self.baud)))
        rospy.loginfo("MaixCam connected via %s", self.port)

    def _disconnect(self):
        if self.ser is not None and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass
        self.ser = None

    def _spin_connected(self):
        """主循环：串口已连接时运行。"""
        rate = rospy.Rate(50)       # 50Hz
        while not rospy.is_shutdown() and self.ser is not None and self.ser.is_open:
            self._send_pending_commands()
            self._read_available_lines()
            rate.sleep()

    def spin(self):
        """节点入口，含自动重连。"""
        while not rospy.is_shutdown():
            try:
                self._connect()
                self._spin_connected()
            except (OSError, serial.SerialException, ConnectionError) as exc:
                self.pub_status.publish(String(data="DISCONNECTED,%s" % exc))
                rospy.logwarn("MaixCam connection lost: %s", exc)
                self._disconnect()
                time.sleep(self.reconnect_delay)
        self._disconnect()


if __name__ == "__main__":
    rospy.init_node("maixcam_uart_bridge")
    MaixCamUartBridgeNode().spin()
