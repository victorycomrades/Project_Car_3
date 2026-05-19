#!/usr/bin/env python3
#coding=UTF-8
 
import rospy
from sensor_msgs.msg import LaserScan
 
def callback(data):
   angle_min=data.angle_min
   range_max=data.range_max
   rospy.loginfo("angle_min:%f range_max:%f",angle_min,range_max)
 
def get_scan():
    rospy.init_node('get_scan', anonymous=True)
    rospy.Subscriber("/scan", LaserScan, callback) 
    rospy.spin()
 
if __name__ == '__main__':
    get_scan()
