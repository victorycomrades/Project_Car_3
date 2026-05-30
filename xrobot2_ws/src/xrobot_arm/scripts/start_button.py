#!/usr/bin/env python
import os
import sys
import rospy
from std_msgs.msg import Bool

try:
    import select
except ImportError:
    select = None


class StartButton(object):
    def __init__(self):
        self.pub = rospy.Publisher('task_start', Bool, queue_size=1)
        self.use_keyboard = rospy.get_param('~use_keyboard', True)
        self.gpio_path = rospy.get_param('~gpio_path', '')
        self.last_state = False

        if self.gpio_path and not os.path.exists(self.gpio_path):
            rospy.logwarn('Start button gpio_path not found: %s', self.gpio_path)
            self.gpio_path = ''

        rospy.loginfo('Start button node initialized: use_keyboard=%s gpio_path=%s', self.use_keyboard, self.gpio_path)

    def publish_start(self):
        self.pub.publish(Bool(data=True))
        rospy.loginfo('task_start published')

    def read_gpio(self):
        try:
            with open(self.gpio_path, 'r') as f:
                value = f.read().strip()
            return value in ['1', 'high', 'true', 'True']
        except Exception as e:
            rospy.logwarn_throttle(30, 'Start button gpio read failed: %s', e)
            return False

    def run(self):
        rate = rospy.Rate(5)
        while not rospy.is_shutdown():
            if self.gpio_path:
                current = self.read_gpio()
                if current and not self.last_state:
                    self.publish_start()
                self.last_state = current

            elif self.use_keyboard and select is not None:
                sys.stdout.write('Press Enter to start...\r')
                sys.stdout.flush()
                r, _, _ = select.select([sys.stdin], [], [], 0.5)
                if r:
                    line = sys.stdin.readline()
                    if line is not None:
                        self.publish_start()

            rate.sleep()


if __name__ == '__main__':
    rospy.init_node('start_button', anonymous=False)
    node = StartButton()
    node.run()
