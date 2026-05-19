#!/usr/bin/env python3
import importlib

import rospy
from sensor_msgs.msg import Joy
import time
from geometry_msgs.msg import Twist, Vector3
from std_msgs.msg import String as StringMsg

class JoyTeleop:
    def __init__(self):
        self.x_speed_scale = rospy.get_param('~x_speed_scale')
        self.y_speed_scale = rospy.get_param('~y_speed_scale')
        self.w_speed_scale = rospy.get_param('~w_speed_scale')
        self.velocity = Twist()
        self.rate = rospy.Rate(20)
        self.active = 0

        self.cmdVelPublisher = rospy.Publisher('cmd_vel', Twist, queue_size = 3)
        self.joySubscriber = rospy.Subscriber('joy', Joy, self.buttonCallback)

    def buttonCallback(self, joy_data):
        self.user_jetson(joy_data)
                        
    def user_jetson(self, joy_data):
        '''
        :jetson joy_data:
            axes 8: [0.0, -0.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0]
            左摇杆(左正右负): axes[0]
            左摇杆(上正下负): axes[1]
            右摇杆(左正右负): axes[2]
            右摇杆(上正下负): axes[3]
            R2(按负抬正): axes[4]
            L2(按负抬正): axes[5]
            左按键(左正右负): axes[6]
            左按键(上正下负): axes[7]
            buttons 15:  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            A: buttons[0]
            B: buttons[1]
            X: buttons[3]
            Y: buttons[4]
            L1: buttons[6]
            R1: buttons[7]
            SELECT: buttons[10]
            START: buttons[11]
            左摇杆按下: buttons[13]
            右摇杆按下: buttons[14]
        '''
        print(joy_data.axes[1], joy_data.axes[0], joy_data.axes[2])
        self.velocity.linear.x = self.x_speed_scale * self.filter_data(joy_data.axes[1])
        self.velocity.linear.y = self.y_speed_scale * self.filter_data(joy_data.axes[0])
        self.velocity.angular.z = self.w_speed_scale * self.filter_data(joy_data.axes[2])
        self.active = 1
        self.cmdVelPublisher.publish(self.velocity)

    def user_pc(self, joy_data):
        '''
        :pc joy_data:
            axes 8: [ -0.0, -0.0, 0.0, -0.0, -0.0, 0.0, 0.0, 0.0 ]
            左摇杆左正右负: axes[0]
            左摇杆上正下负: axes[1]
            L2按负抬正:  axes[2]
            右摇杆左正右负: axes[3]
            右摇杆上正下负: axes[4]
            R2按负抬正:  axes[5]
            左按键左正右负: axes[6]
            左按键上正下负: axes[7]
            buttons 11: [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
            A: buttons[0]
            B: buttons[1]
            X: buttons[2]
            Y: buttons[3]
            L1: buttons[4]
            R1: buttons[5]
            SELECT: buttons[6]
            MODE: buttons[7]
            START: buttons[8]
            左摇杆按下: buttons[9]
            右摇杆按下: buttons[10]
        '''
        self.velocity.linear.x = self.x_speed_scale * self.filter_data(joy_data.axes[1])
        self.velocity.linear.y = self.y_speed_scale * self.filter_data(joy_data.axes[0])
        self.velocity.angular.z = self.w_speed_scale * self.filter_data(joy_data.axes[3])
        self.active = 1
        self.cmdVelPublisher.publish(self.velocity)
        
    def filter_data(self, value):
        if abs(value) < 0.2: value = 0
        return value


	
if __name__ == '__main__':
	rospy.init_node('joy_teleop')
	joy = JoyTeleop()
	try:
		rospy.spin()
	except	rospy.ROSInterruptException:
		print('exception')