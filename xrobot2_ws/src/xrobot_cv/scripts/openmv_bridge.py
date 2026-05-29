#!/usr/bin/env python
import rospy
from std_msgs.msg import String
from geometry_msgs.msg import PoseStamped
try:
    import serial
except ImportError:
    serial = None


class OpenMVBridge(object):
    def __init__(self):
        self.port = rospy.get_param('~port', '/dev/ttyACM0')
        self.baud = rospy.get_param('~baud', 115200)
        self.use_serial = rospy.get_param('~use_serial', True)
        self.input_topic = rospy.get_param('~input_topic', '')

        self.task_pub = rospy.Publisher('openmv/task_code', String, queue_size=1)
        self.object_pose_pub = rospy.Publisher('openmv/object_pose', PoseStamped, queue_size=1)
        self.object_color_pub = rospy.Publisher('openmv/object_color', String, queue_size=1)
        self.ring_pose_pub = rospy.Publisher('openmv/color_ring_pose', PoseStamped, queue_size=1)
        self.ring_color_pub = rospy.Publisher('openmv/color_ring_color', String, queue_size=1)
        self.display_pub = rospy.Publisher('openmv/display_text', String, queue_size=1)

        self.serial_port = None
        if self.use_serial and serial is not None:
            try:
                self.serial_port = serial.Serial(self.port, self.baud, timeout=0.1)
                rospy.loginfo('OpenMV bridge connected to %s @ %d', self.port, self.baud)
            except Exception as e:
                rospy.logwarn('OpenMV bridge failed to open serial port %s: %s', self.port, e)
                self.serial_port = None
        elif self.use_serial:
            rospy.logwarn('pyserial not installed; OpenMV serial bridge disabled')

        if self.input_topic:
            rospy.Subscriber(self.input_topic, String, self.receive_line)

    def receive_line(self, msg):
        self.parse_and_publish(msg.data.strip())

    def read_serial(self):
        if not self.serial_port:
            return
        try:
            raw = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
            if raw:
                rospy.loginfo('RAW: %s', raw)
                self.parse_and_publish(raw)
        except Exception as e:
            rospy.logwarn_throttle(10, 'OpenMV serial read error: %s', e)

    def parse_and_publish(self, raw):
        if raw.startswith('QR:'):
            value = raw[3:].strip()
            self.task_pub.publish(String(data=value))
            self.display_pub.publish(String(data=value))
            rospy.loginfo('OpenMV QR code -> %s', value)
            return

        if raw.startswith('OBJ:'):
            parts = raw[4:].split(',')
            if len(parts) >= 4:
                obj_type = parts[0].strip()
                color = parts[1].strip()
                x = float(parts[2].strip())
                y = float(parts[3].strip())
                pose = PoseStamped()
                pose.header.stamp = rospy.Time.now()
                pose.header.frame_id = 'openmv_camera'
                pose.pose.position.x = x
                pose.pose.position.y = y
                pose.pose.position.z = float(parts[4].strip()) if len(parts) > 4 else 0.0
                self.object_pose_pub.publish(pose)
                self.object_color_pub.publish(String(data=color))
                rospy.loginfo('OpenMV object %s %s @ %.1f, %.1f', obj_type, color, x, y)
            return

        if raw.startswith('RING:'):
            parts = raw[5:].split(',')
            if len(parts) >= 3:
                color = parts[0].strip()
                x = float(parts[1].strip())
                y = float(parts[2].strip())
                pose = PoseStamped()
                pose.header.stamp = rospy.Time.now()
                pose.header.frame_id = 'openmv_camera'
                pose.pose.position.x = x
                pose.pose.position.y = y
                self.ring_pose_pub.publish(pose)
                self.ring_color_pub.publish(String(data=color))
                rospy.loginfo('OpenMV ring %s @ %.1f, %.1f', color, x, y)
            return

        if raw.startswith('LED:'):
            self.display_pub.publish(String(data=raw[4:].strip()))
            return

        rospy.logdebug('OpenMV raw line ignored: %s', raw)

    def run(self):
        rate = rospy.Rate(20)
        while not rospy.is_shutdown():
            if self.use_serial:
                self.read_serial()
            rate.sleep()


if __name__ == '__main__':
    rospy.init_node('openmv_bridge', anonymous=False)
    bridge = OpenMVBridge()
    bridge.run()
