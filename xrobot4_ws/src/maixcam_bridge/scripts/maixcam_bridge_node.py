#!/usr/bin/env python3
import queue
import socket
import time

import rospy
from geometry_msgs.msg import PointStamped, Pose2D
from std_msgs.msg import String

from maixcam_bridge.protocol import format_mode_command, parse_line


class MaixCamBridgeNode:
    def __init__(self):
        self.host = rospy.get_param("~host", "10.131.167.1")
        self.port = int(rospy.get_param("~port", 7000))
        self.topic_prefix = rospy.get_param("~topic_prefix", "/vision").rstrip("/")
        self.frame_id = rospy.get_param("~frame_id", "maixcam")
        self.reconnect_delay = float(rospy.get_param("~reconnect_delay", 1.0))

        self.command_queue = queue.Queue()
        self.sock = None
        self.rx_buffer = ""

        self.pub_raw = rospy.Publisher(self.topic_prefix + "/raw", String, queue_size=20)
        self.pub_status = rospy.Publisher(self.topic_prefix + "/status", String, queue_size=5, latch=True)
        self.pub_qrcode = rospy.Publisher(self.topic_prefix + "/qrcode", String, queue_size=5)
        self.pub_blob_raw = rospy.Publisher(self.topic_prefix + "/blob/raw", String, queue_size=10)
        self.pub_blob_center = rospy.Publisher(self.topic_prefix + "/blob/center", PointStamped, queue_size=10)
        self.pub_ring_raw = rospy.Publisher(self.topic_prefix + "/ring/raw", String, queue_size=10)
        self.pub_ring_center = rospy.Publisher(self.topic_prefix + "/ring/center", PointStamped, queue_size=10)
        self.pub_line_raw = rospy.Publisher(self.topic_prefix + "/line/raw", String, queue_size=10)
        self.pub_line = rospy.Publisher(self.topic_prefix + "/line", Pose2D, queue_size=10)
        self.pub_none = rospy.Publisher(self.topic_prefix + "/none", String, queue_size=10)

        rospy.Subscriber(self.topic_prefix + "/mode", String, self.mode_callback, queue_size=10)

    def mode_callback(self, msg):
        command = msg.data.strip()
        if command and not command.upper().startswith("MODE,"):
            command = "MODE," + command
        if command:
            self.command_queue.put(command)

    def connect(self):
        self.close()
        rospy.loginfo("Connecting to MaixCam at %s:%d", self.host, self.port)
        sock = socket.create_connection((self.host, self.port), timeout=3.0)
        sock.settimeout(0.05)
        self.sock = sock
        self.rx_buffer = ""
        self.publish_status("CONNECTED,%s,%d" % (self.host, self.port))
        rospy.loginfo("MaixCam connected")

    def close(self):
        if self.sock is not None:
            try:
                self.sock.close()
            except OSError:
                pass
        self.sock = None

    def publish_status(self, text):
        self.pub_status.publish(String(data=text))

    def publish_point(self, publisher, msg, z_field):
        point = PointStamped()
        point.header.stamp = rospy.Time.now()
        point.header.frame_id = self.frame_id
        point.point.x = float(msg.fields.get("cx", 0))
        point.point.y = float(msg.fields.get("cy", 0))
        point.point.z = float(msg.fields.get(z_field, 0))
        publisher.publish(point)

    def publish_parsed(self, line):
        self.pub_raw.publish(String(data=line))
        try:
            msg = parse_line(line)
        except ValueError as exc:
            rospy.logwarn("Ignoring malformed MaixCam line %r: %s", line, exc)
            return

        if msg.kind == "QR":
            self.pub_qrcode.publish(String(data=msg.payload or ""))
        elif msg.kind == "BLOB":
            self.pub_blob_raw.publish(String(data=msg.raw))
            self.publish_point(self.pub_blob_center, msg, "area")
        elif msg.kind == "RING":
            self.pub_ring_raw.publish(String(data=msg.raw))
            self.publish_point(self.pub_ring_center, msg, "radius")
        elif msg.kind == "LINE":
            pose = Pose2D()
            pose.x = float(msg.fields["dx"])
            pose.y = 0.0
            pose.theta = float(msg.fields["theta"])
            self.pub_line_raw.publish(String(data=msg.raw))
            self.pub_line.publish(pose)
        elif msg.kind == "NONE":
            self.pub_none.publish(String(data=msg.raw))
        elif msg.kind in ("HELLO", "INFO", "WARN", "ERR"):
            self.publish_status(msg.raw)

    def send_pending_commands(self):
        while not self.command_queue.empty():
            command = self.command_queue.get_nowait()
            self.sock.sendall(format_mode_command(command))
            rospy.loginfo("Sent MaixCam command: %s", command)

    def read_available_lines(self):
        try:
            chunk = self.sock.recv(1024)
        except socket.timeout:
            return
        if not chunk:
            raise ConnectionError("MaixCam closed the TCP connection")

        self.rx_buffer += chunk.decode("utf-8", errors="replace")
        while "\n" in self.rx_buffer:
            line, self.rx_buffer = self.rx_buffer.split("\n", 1)
            line = line.strip()
            if line:
                self.publish_parsed(line)

    def spin_connected(self):
        rate = rospy.Rate(50)
        while not rospy.is_shutdown() and self.sock is not None:
            self.send_pending_commands()
            self.read_available_lines()
            rate.sleep()

    def spin(self):
        while not rospy.is_shutdown():
            try:
                self.connect()
                self.spin_connected()
            except (OSError, ConnectionError) as exc:
                self.publish_status("DISCONNECTED,%s" % exc)
                rospy.logwarn("MaixCam connection lost: %s", exc)
                self.close()
                time.sleep(self.reconnect_delay)
        self.close()


if __name__ == "__main__":
    rospy.init_node("maixcam_bridge")
    MaixCamBridgeNode().spin()
