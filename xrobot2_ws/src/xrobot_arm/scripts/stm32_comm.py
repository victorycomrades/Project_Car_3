#!/usr/bin/env python
import rospy
import serial
import json
import math
import time
from std_msgs.msg import String, Bool
from geometry_msgs.msg import PoseStamped, Twist
from nav_msgs.msg import Odometry


class STM32Comm:
    def __init__(self):
        self.port_name = rospy.get_param('~port', '/dev/ttyUSB0')
        self.baud = rospy.get_param('~baud', 115200)
        self.reconnect_delay = rospy.get_param('~reconnect_delay', 2.0)
        self.serial_port = None
        self.estop_active = False
        self.last_cmd_vel_time = rospy.Time.now()
        self.watchdog_timeout = rospy.Duration(0.5)
        self._connect_serial()

        self.arm_target_sub = rospy.Subscriber('/arm_target_pose', PoseStamped, self.arm_target_cb)
        self.cmd_vel_sub = rospy.Subscriber('/cmd_vel', Twist, self.cmd_vel_cb)
        self.display_sub = rospy.Subscriber('openmv/display_text', String, self.display_cb)
        self.estop_sub = rospy.Subscriber('/estop', Bool, self.estop_cb)

        self.arm_status_pub = rospy.Publisher('/arm_status', String, queue_size=1)
        self.odom_pub = rospy.Publisher('/odom', Odometry, queue_size=10)
        self.wheel_pub = rospy.Publisher('/wheel_data', String, queue_size=10)

    def _connect_serial(self):
        while self.serial_port is None and not rospy.is_shutdown():
            try:
                self.serial_port = serial.Serial(self.port_name, self.baud, timeout=1, dsrdtr=False)
                rospy.loginfo('STM32 serial connected: %s @ %d', self.port_name, self.baud)
            except (serial.SerialException, OSError) as e:
                rospy.logwarn('Serial connection failed: %s. Retrying in %.1fs...', str(e), self.reconnect_delay)
                time.sleep(self.reconnect_delay)

    def _ensure_connected(self):
        if self.serial_port is None or not self.serial_port.is_open:
            try:
                self.serial_port.close()
            except Exception:
                pass
            self.serial_port = None
            self._connect_serial()

    def arm_target_cb(self, msg):
        data = {
            'cmd': 'arm',
            'x': msg.pose.position.x,
            'y': msg.pose.position.y,
            'z': msg.pose.position.z
        }
        self.send_to_stm32(json.dumps(data, separators=(',', ':')))

    def cmd_vel_cb(self, msg):
        self.last_cmd_vel_time = rospy.Time.now()
        if self.estop_active:
            self.send_zero_vel()
            return
        data = {
            'cmd': 'nav',
            'linear_x': msg.linear.x,
            'linear_y': msg.linear.y,
            'angular_z': msg.angular.z
        }
        self.send_to_stm32(json.dumps(data, separators=(',', ':')))

    def display_cb(self, msg):
        text = msg.data.strip()
        rospy.loginfo('STM32 LED display: %s', text)
        data = {
            'cmd': 'display',
            'text': text
        }
        self.send_to_stm32(json.dumps(data, separators=(',', ':')))

    def estop_cb(self, msg):
        if msg.data:
            self.estop_active = True
            self.send_zero_vel()
            rospy.logwarn('EMERGENCY STOP activated')
        else:
            self.estop_active = False
            rospy.loginfo('Emergency stop released')

    def send_zero_vel(self):
        data = {
            'cmd': 'nav',
            'linear_x': 0.0,
            'linear_y': 0.0,
            'angular_z': 0.0
        }
        self.send_to_stm32(json.dumps(data, separators=(',', ':')))

    def send_to_stm32(self, msg):
        self._ensure_connected()
        if self.serial_port is None:
            return
        try:
            self.serial_port.write((msg + '\n').encode())
        except (serial.SerialException, OSError) as e:
            rospy.logerr('Serial send error: %s', str(e))
            self.serial_port = None

    def read_from_stm32(self):
        self._ensure_connected()
        if self.serial_port is None:
            return
        try:
            line = self.serial_port.readline().decode().strip()
            if line:
                data = json.loads(line)
                if 'status' in data:
                    self.arm_status_pub.publish(String(data=data['status']))
                if 'odom' in data:
                    self.handle_odom(data['odom'])
                if 'wheel' in data:
                    self.handle_wheel(data['wheel'])
        except (serial.SerialException, OSError):
            self.serial_port = None
        except (ValueError, UnicodeDecodeError):
            pass

    def handle_odom(self, odom_data):
        odom = Odometry()
        odom.header.stamp = rospy.Time.now()
        odom.header.frame_id = 'odom'
        odom.child_frame_id = 'base_footprint'

        odom.pose.pose.position.x = odom_data.get('x', 0.0)
        odom.pose.pose.position.y = odom_data.get('y', 0.0)
        odom.pose.pose.position.z = 0.0

        theta = odom_data.get('theta', 0.0)
        odom.pose.pose.orientation.z = math.sin(theta / 2.0)
        odom.pose.pose.orientation.w = math.cos(theta / 2.0)

        odom.pose.covariance[0] = 0.01
        odom.pose.covariance[7] = 0.01
        odom.pose.covariance[14] = 99999.0
        odom.pose.covariance[21] = 99999.0
        odom.pose.covariance[28] = 99999.0
        odom.pose.covariance[35] = 0.01

        odom.twist.twist.linear.x = odom_data.get('vx', 0.0)
        odom.twist.twist.linear.y = odom_data.get('vy', 0.0)
        odom.twist.twist.angular.z = odom_data.get('vtheta', 0.0)

        odom.twist.covariance[0] = 0.01
        odom.twist.covariance[7] = 0.01
        odom.twist.covariance[14] = 99999.0
        odom.twist.covariance[21] = 99999.0
        odom.twist.covariance[28] = 99999.0
        odom.twist.covariance[35] = 0.01

        self.odom_pub.publish(odom)

    def handle_wheel(self, wheel_data):
        x = wheel_data.get('x', 0.0)
        y = wheel_data.get('y', 0.0)
        zangle = wheel_data.get('zangle', 0.0)
        ready = wheel_data.get('ready', 0)
        rospy.loginfo('WHEEL: x=%.2f mm, y=%.2f mm, zangle=%.2f deg, ready=%d', x, y, zangle, ready)
        self.wheel_pub.publish(String(data='x=%.2f,y=%.2f,zangle=%.2f,ready=%d' % (x, y, zangle, ready)))

    def check_watchdog(self):
        if rospy.Time.now() - self.last_cmd_vel_time > self.watchdog_timeout:
            self.send_zero_vel()

    def run(self):
        rate = rospy.Rate(10)
        while not rospy.is_shutdown():
            self.read_from_stm32()
            self.check_watchdog()
            rate.sleep()


if __name__ == '__main__':
    rospy.init_node('stm32_comm')
    comm = STM32Comm()
    comm.run()
