#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021 PS-Micro, Co. Ltd.
#
# SPDX-License-Identifier: Apache-2.0
#

import time

import rospy
import cv2
from cv_bridge import CvBridge, CvBridgeError
from sensor_msgs.msg import Image
import numpy as np
from math import *
from geometry_msgs.msg import Pose
from dynamic_reconfigure.server import Server
from xrobot_cv.cfg import Params_colorConfig



class ImageConverter:
    def __init__(self):
        # 创建图像缓存相关的变量
        self.cv_image = None
        self.get_image = False
        self.srv=Server(Params_colorConfig,self.callback1)
        
        self.HUE_LOW =35
        self.SATURATION_LOW=43
        self.VALUE_LOW =46
        self.HUE_HIGH =77
        self.SATURATION_HIGH =255
        self.VALUE_HIGH =255
        


        # 创建cv_bridge
        self.bridge = CvBridge()

        # 声明图像的发布者和订阅者
        self.image_pub = rospy.Publisher("object_detect_image",
                                         Image,
                                         queue_size=1)
        self.target_pub = rospy.Publisher("object_detect_pose",
                                          Pose,
                                          queue_size=1)
        image_topic = rospy.get_param("~image_topic",
                                      "/camera/image_raw")
        rospy.loginfo("object_detect subscribe image topic: %s", image_topic)
        self.image_sub = rospy.Subscriber(image_topic,
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

    def detect_object(self):
        # 创建HSV阈值列表
        boundaries = [[[self.HUE_LOW, self.SATURATION_LOW,
                        self.VALUE_LOW], [self.HUE_HIGH, self.SATURATION_HIGH, self.VALUE_HIGH]]]

        # 遍历HSV阈值列表
        for (lower, upper) in boundaries:
            # 创建HSV上下限位的阈值数组
            lower = np.array(lower, dtype="uint8")
            upper = np.array(upper, dtype="uint8")

        # 高斯滤波，对图像邻域内像素进行平滑
        hsv_image = cv2.GaussianBlur(self.cv_image, (5, 5), 0)

        # 颜色空间转换，将RGB图像转换成HSV图像
        hsv_image = cv2.cvtColor(hsv_image, cv2.COLOR_BGR2HSV)
        
        erode_hsv = cv2.erode(hsv_image, None, iterations=2)##
        
        # 根据阈值，去除背景
        mask = cv2.inRange(erode_hsv, lower, upper)
        output = cv2.bitwise_and(self.cv_image, self.cv_image, mask=mask)

        # 将彩色图像转换成灰度图像
        cvImg = cv2.cvtColor(output, 6)  # cv2.COLOR_BGR2GRAY
        npImg = np.asarray(cvImg)
        thresh = cv2.threshold(npImg, 1, 255, cv2.THRESH_BINARY)[1]

        # 检测目标物体的轮廓
        #findcon_img, cnts, hierarchy = cv2.findContours(thresh, cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)
        cnts, hierarchy = cv2.findContours(thresh, cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)
        
                
        #for contour in cnts: # 获取包围轮廓的最小旋转矩形 
            #if contour.shape[0] < 150:
                #continue
           # rect = cv2.minAreaRect(contour) 
           # box = cv2.boxPoints(rect) 
            #box = np.int0(box) # 计算矩形中心点 
            #cX = int((box[0][0] + box[2][0]) / 2) 
            #cY = int((box[0][1] + box[2][1]) / 2) # 绘制旋转矩形和中心点 
            #cv2.drawContours(self.cv_image, [box], 0, (255, 0, 0), 2) 
            #cv2.circle(self.cv_image, (cX, cY), 5, (0, 0, 255), -1)

        # 遍历找到的所有轮廓线
        for c in cnts:

            # 去除一些面积太小的噪声
            if c.shape[0] < 150:
                continue

            # 提取轮廓的特征
            M = cv2.moments(c)

            if int(M["m00"]) < 500:
                continue

            cX = int(M["m10"] / M["m00"])
            cY = int(M["m01"] / M["m00"])

            print("x: {}, y: {}, size: {}".format(cX, cY, M["m00"]))

            # 把轮廓描绘出来，并绘制中心点
            cv2.drawContours(self.cv_image, [c], -1, (0, 0, 255), 2)
            x, y, w, h = cv2.boundingRect(c)
            cv2.rectangle(self.cv_image, (x, y), (x + w, y + h),
                          (255, 0, 0), 2)
            cv2.circle(self.cv_image, (cX, cY), 4, (0, 0, 255), -1)
            
            # 将目标位置通过话题发布
            objPose = Pose()
            objPose.position.x = cX
            objPose.position.y = cY
            objPose.position.z = M["m00"]
            self.target_pub.publish(objPose)

        # 再将opencv格式额数据转换成ros image格式的数据发布
        try:
            self.image_pub.publish(
                self.bridge.cv2_to_imgmsg(self.cv_image, "bgr8"))
        except CvBridgeError as e:
            print (e)

    def loop(self):
        if self.get_image:
            self.detect_object()
            self.get_image = False
    def callback1(self, config, level):


        self.HUE_LOW =config['Hmin']
        self.SATURATION_LOW=config['Smin']
        self.VALUE_LOW =config['Vmin']
        self.HUE_HIGH =config['Hmax']
        self.SATURATION_HIGH =config['Smax']
        self.VALUE_HIGH =config['Vmax']

			
        return  config


if __name__ == '__main__':
    try:
        # 初始化ros节点
        rospy.init_node("object_detect")
        rospy.loginfo("Starting detect object")
        image_converter = ImageConverter()
        rate = rospy.Rate(100)
        while not rospy.is_shutdown():
            image_converter.loop()
            rate.sleep()
    except KeyboardInterrupt:
        print ("Shutting down object_detect node.")
        cv2.destroyAllWindows()
