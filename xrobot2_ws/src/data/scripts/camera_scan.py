#!/usr/bin/env python3
#coding=UTF-8
 
import rospy
from sensor_msgs.msg import CameraInfo
 
def callback(data):
   height=data.height
   width=data.width
   rospy.loginfo("height:%f width:%f",height,width)
def get_camera():
    rospy.init_node('get_camera', anonymous=True)
    rospy.Subscriber("/camera/depth/camera_info", CameraInfo, callback) 
    rospy.spin()
 
if __name__ == '__main__':
    get_camera()
