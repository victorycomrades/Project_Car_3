#!/usr/bin/env python3
# encoding: utf-8

import rospy
import cv2
import numpy as np
from time import *

from cv_bridge import CvBridge, CvBridgeError
from sensor_msgs.msg import Image
from math import *
import serial
from jetarm import arm_v1servo

ser=serial.Serial(port="/dev/XCOM1",baudrate=1000000)



class ImageConverter:
    def __init__(self):
        # 创建图像缓存相关的变量
        self.cv_image = None
        self.get_image = False
        self.arm=arm_v1servo.Arm_v1servo(ser,lower_arm_servo_reset_angle=0)
        self.arm.angle_control(0,-5,-45,90,0,0,50,100)

        # 创建cv_bridge
        self.bridge = CvBridge()
        self.image_pub = rospy.Publisher("detect_image",
                                         Image,
                                         queue_size=1)
        self.image_sub = rospy.Subscriber("/camera/color/image_raw",
                                          Image,
                                          self.callback,
                                          queue_size=1)

    def callback(self, data):
        # 判断当前图像是否处理完
        if not self.get_image:
            # 使用cv_bridge将ROS的图像数据转换成OpenCV的图像格式
            try:
                self.cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
            except CvBridgeError as e:
                print (e)
            # 设置标志，表示收到图像
            self.get_image = True

 
    def color_detection(self):
        # 定义颜色范围（在HSV颜色空间中）
        lower_red = np.array([0, 100, 100])
        upper_red = np.array([10, 255, 255])
        lower_blue = np.array([110, 100, 100])
        upper_blue = np.array([130, 255, 255])
        lower_green = np.array([50, 100, 100])
        upper_green = np.array([70, 255, 255])
 
        # 将帧转换为HSV颜色空间
        hsv_frame = cv2.cvtColor(self.cv_image, cv2.COLOR_BGR2HSV)
 
        # 根据颜色范围创建掩膜
        red_mask = cv2.inRange(hsv_frame, lower_red, upper_red)
        blue_mask = cv2.inRange(hsv_frame, lower_blue, upper_blue)
        green_mask = cv2.inRange(hsv_frame, lower_green, upper_green)
 
        # 对掩膜进行形态学操作，以去除噪声
        kernel = np.ones((5, 5), np.uint8)
        red_mask = cv2.morphologyEx(red_mask, cv2.MORPH_OPEN, kernel)
        blue_mask = cv2.morphologyEx(blue_mask, cv2.MORPH_OPEN, kernel)
        green_mask = cv2.morphologyEx(green_mask, cv2.MORPH_OPEN, kernel)
 
        # 在原始帧中找到颜色区域并绘制方框
        contours, _ = cv2.findContours(red_mask + blue_mask + green_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        for contour in contours:
            x, y, w, h = cv2.boundingRect(contour)
            if cv2.contourArea(contour) > 500:  # 设置最小区域面积以排除噪声
                if np.any(red_mask[y:y+h, x:x+w]):
                    cv2.drawContours(self.cv_image,[contour],-1,(0,0,255),2)
                    cv2.rectangle(self.cv_image, (x, y), (x+w, y+h), (0, 0, 255), 2)
                    self.image_pub.publish(self.bridge.cv2_to_imgmsg(self.cv_image, "bgr8"))
                    sleep(3)
                    self.arm.angle_control(0,-5,-45,24,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(0,54,-50,24,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(0,45,-60,90,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(0,30,-40,90,0,0,50,200)
                elif np.any(blue_mask[y:y+h, x:x+w]):
                    cv2.rectangle(self.cv_image, (x, y), (x+w, y+h), (255, 0, 0), 2)
                    cv2.drawContours(self.cv_image,[contour],-1,(255, 0, 0),2)
                    self.image_pub.publish(self.bridge.cv2_to_imgmsg(self.cv_image, "bgr8"))
                    sleep(3)
                    self.arm.angle_control(0,-5,-45,24,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(40,54,-50,24,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(40,45,-60,90,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(40,30,-40,90,0,0,50,200)
                elif np.any(green_mask[y:y+h, x:x+w]):
                    cv2.rectangle(self.cv_image, (x, y), (x+w, y+h), (0, 255, 0), 2)
                    cv2.drawContours(self.cv_image,[contour],-1,(0, 255, 0),2)
                    self.image_pub.publish(self.bridge.cv2_to_imgmsg(self.cv_image, "bgr8"))
                    sleep(3)
                    self.arm.angle_control(0,-5,-45,24,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(-40,54,-50,24,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(-40,45,-60,90,0,0,50,200)
                    sleep(6)
                    self.arm.angle_control(-40,30,-40,90,0,0,50,200)
            else:
                self.image_pub.publish(self.bridge.cv2_to_imgmsg(self.cv_image, "bgr8"))
        self.arm.angle_control(0,-5,-45,90,0,0,50,100)

 

    
    def loop(self):
        if self.get_image:
            self.color_detection()
            self.get_image = False
    
if __name__ == '__main__':
    try:
        # 初始化ros节点
        rospy.init_node("object_detect")
        image_converter = ImageConverter()
        rate = rospy.Rate(100)
        while not rospy.is_shutdown():
            image_converter.loop()
            rate.sleep()
    except KeyboardInterrupt:
        print ("Shutting down object_detect node.")
        cv2.destroyAllWindows()

 
