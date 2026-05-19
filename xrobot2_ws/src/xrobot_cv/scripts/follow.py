#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
import cv2
from cv_bridge import CvBridge, CvBridgeError
from sensor_msgs.msg import Image
import numpy as np
from math import *
from geometry_msgs.msg import Twist
from geometry_msgs.msg import Pose



objPose = Pose()
objPose.position.x = 0
objPose.position.y = 0
objPose.position.z = 0

#vel = Twist()
#vel.linear.x = 0.0
#vel.linear.y = 0.0
#vel.linear.z = 0.0
#vel.angular.x = 0.0
#vel.angular.y = 0.0
#vel.angular.z = 0.0


class follow_object:
    def __init__(self):    
        #订阅位姿信息

        self.Pose_sub = rospy.Subscriber("object_detect_pose", Pose, self.poseCallback)
        #发布速度指令
        self.vel_pub = rospy.Publisher('cmd_vel', Twist, queue_size=5)
        self.target_area = rospy.get_param("~target_area", 15000.0)
        self.area_deadband = rospy.get_param("~area_deadband", 1000.0)
        self.center_x = rospy.get_param("~center_x", 320.0)
        self.center_deadband = rospy.get_param("~center_deadband", 10.0)
        self.max_linear_speed = rospy.get_param("~max_linear_speed", 0.25)
        self.max_angular_speed = rospy.get_param("~max_angular_speed", 0.8)
        self.linear_gain = rospy.get_param("~linear_gain", 0.00003)
        self.angular_gain = rospy.get_param("~angular_gain", 0.004)


    def poseCallback(self,Pose):

        X = Pose.position.x;
        Y = Pose.position.y;
        Z = Pose.position.z;


        vel = Twist()

        area_error = self.target_area - Z
        if abs(area_error) > self.area_deadband:
            vel.linear.x = area_error * self.linear_gain
            vel.linear.x = max(-self.max_linear_speed,
                               min(self.max_linear_speed, vel.linear.x))

        center_error = self.center_x - X
        if abs(center_error) > self.center_deadband:
            vel.angular.z = center_error * self.angular_gain
            vel.angular.z = max(-self.max_angular_speed,
                                min(self.max_angular_speed, vel.angular.z))

        self.vel_pub.publish(vel)
        rospy.loginfo(
            "Publish velocity command[{} m/s, {} rad/s], target[x={}, area={}]".
            format(vel.linear.x, vel.angular.z, X, Z))

if __name__ == '__main__':
    try:
        # 初始化ros节点
        rospy.init_node("follow_object")
        rospy.loginfo("Starting follow object")
        follow_object()
        rospy.spin()
    except KeyboardInterrupt:
        print ("Shutting down follow_object node.")
        cv2.destroyAllWindows()
