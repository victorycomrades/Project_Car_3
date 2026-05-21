#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Bridge MaixCam/OpenMV UART text messages into ROS topics."""

import threading

import rospy
from geometry_msgs.msg import PoseStamped
from std_msgs.msg import String

try:
    import serial
except ImportError:
    serial = None


class OpenMVBridge(object):
    def __init__(self):
        self.port = rospy.get_param("~port", "/dev/ttyUSB0")
        self.baud = int(rospy.get_param("~baud", 115200))
        self.use_serial = bool(rospy.get_param("~use_serial", True))
        self.input_topic = rospy.get_param("~input_topic", "")
        self.startup_mode = rospy.get_param("~startup_mode", "")

        self.task_pub = rospy.Publisher("openmv/task_code", String, queue_size=1)
        self.object_pose_pub = rospy.Publisher("openmv/object_pose", PoseStamped, queue_size=1)
        self.object_color_pub = rospy.Publisher("openmv/object_color", String, queue_size=1)
        self.ring_pose_pub = rospy.Publisher("openmv/color_ring_pose", PoseStamped, queue_size=1)
        self.ring_color_pub = rospy.Publisher("openmv/color_ring_color", String, queue_size=1)
        self.display_pub = rospy.Publisher("openmv/display_text", String, queue_size=1)
        self.raw_pub = rospy.Publisher("openmv/raw", String, queue_size=10)
        self.status_pub = rospy.Publisher("openmv/status", String, queue_size=5, latch=True)

        self.serial_port = None
        self.serial_lock = threading.Lock()
        if self.use_serial and serial is not None:
            self.open_serial()
        elif self.use_serial:
            rospy.logwarn("pyserial not installed; serial bridge disabled")

        if self.input_topic:
            rospy.Subscriber(self.input_topic, String, self.receive_line)
        rospy.Subscriber("openmv/mode", String, self.mode_callback, queue_size=5)

    def open_serial(self):
        try:
            self.serial_port = serial.Serial(self.port, self.baud, timeout=0.1)
            rospy.loginfo("OpenMV/MaixCam bridge connected to %s @ %d", self.port, self.baud)
            self.status_pub.publish(String(data="CONNECTED,%s,%d" % (self.port, self.baud)))
            if self.startup_mode:
                self.send_command(self.startup_mode)
        except Exception as exc:
            rospy.logwarn("Failed to open serial port %s: %s", self.port, exc)
            self.status_pub.publish(String(data="DISCONNECTED,%s" % exc))
            self.serial_port = None

    def mode_callback(self, msg):
        command = msg.data.strip()
        if command:
            self.send_command(command)

    def send_command(self, command):
        if not self.serial_port:
            rospy.logwarn_throttle(5, "No serial port open; cannot send %s", command)
            return
        text = command.strip()
        if not text.upper().startswith("MODE,") and text.upper() != "PING":
            text = "MODE," + text
        try:
            with self.serial_lock:
                self.serial_port.write((text + "\n").encode("utf-8"))
            rospy.loginfo("Sent to MaixCam: %s", text)
        except Exception as exc:
            rospy.logwarn("Serial write failed: %s", exc)

    def receive_line(self, msg):
        self.parse_and_publish(msg.data.strip())

    def read_serial(self):
        if not self.serial_port:
            return
        try:
            raw = self.serial_port.readline().decode("utf-8", errors="ignore").strip()
            if raw:
                self.parse_and_publish(raw)
        except Exception as exc:
            rospy.logwarn_throttle(10, "Serial read error: %s", exc)

    def parse_and_publish(self, raw):
        self.raw_pub.publish(String(data=raw))
        if raw.startswith("QR:"):
            self.publish_qr(raw[3:].strip())
            return
        if raw.startswith("OBJ:"):
            self.parse_legacy_obj(raw[4:])
            return
        if raw.startswith("RING:"):
            self.parse_legacy_ring(raw[5:])
            return
        if raw.startswith("LED:"):
            self.display_pub.publish(String(data=raw[4:].strip()))
            return

        parts = [part.strip() for part in raw.split(",")]
        if not parts:
            return
        kind = parts[0].upper()

        if kind == "QR" and len(parts) >= 2:
            self.publish_qr(parts[1])
        elif kind == "BLOB" and len(parts) >= 9:
            color = parts[1].upper()
            self.publish_pose(self.object_pose_pub, "maixcam", parts[2], parts[3], parts[8])
            self.object_color_pub.publish(String(data=color))
            rospy.loginfo("MaixCam BLOB %s dx=%s dy=%s area=%s", color, parts[2], parts[3], parts[8])
        elif kind == "RING" and len(parts) >= 7:
            color = parts[1].upper()
            self.publish_pose(self.ring_pose_pub, "maixcam", parts[2], parts[3], parts[6])
            self.ring_color_pub.publish(String(data=color))
            rospy.loginfo("MaixCam RING %s dx=%s dy=%s radius=%s", color, parts[2], parts[3], parts[6])
        elif kind == "LINE":
            self.status_pub.publish(String(data=raw))
        elif kind in ("HELLO", "INFO", "WARN", "ERR", "PONG", "NONE"):
            self.status_pub.publish(String(data=raw))
            rospy.loginfo("MaixCam status: %s", raw)
        else:
            rospy.logdebug("Ignored camera line: %s", raw)

    def publish_qr(self, value):
        self.task_pub.publish(String(data=value))
        self.display_pub.publish(String(data=value))
        rospy.loginfo("Camera QR code -> %s", value)

    def parse_legacy_obj(self, payload):
        parts = payload.split(",")
        if len(parts) < 4:
            return
        color = parts[1].strip()
        z_value = parts[4].strip() if len(parts) > 4 else "0"
        self.publish_pose(self.object_pose_pub, "openmv_camera", parts[2], parts[3], z_value)
        self.object_color_pub.publish(String(data=color))

    def parse_legacy_ring(self, payload):
        parts = payload.split(",")
        if len(parts) < 3:
            return
        color = parts[0].strip()
        self.publish_pose(self.ring_pose_pub, "openmv_camera", parts[1], parts[2], "0")
        self.ring_color_pub.publish(String(data=color))

    @staticmethod
    def publish_pose(publisher, frame_id, x_value, y_value, z_value):
        try:
            x = float(x_value)
            y = float(y_value)
            z = float(z_value)
        except ValueError:
            return
        pose = PoseStamped()
        pose.header.stamp = rospy.Time.now()
        pose.header.frame_id = frame_id
        pose.pose.position.x = x
        pose.pose.position.y = y
        pose.pose.position.z = z
        publisher.publish(pose)

    def run(self):
        rate = rospy.Rate(20)
        while not rospy.is_shutdown():
            if self.use_serial:
                self.read_serial()
            rate.sleep()


if __name__ == "__main__":
    rospy.init_node("openmv_bridge", anonymous=False)
    OpenMVBridge().run()
