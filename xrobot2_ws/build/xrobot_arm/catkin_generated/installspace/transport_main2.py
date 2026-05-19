#!/usr/bin/env python3
# encoding: utf-8
import rospy
import cv2 as cv
from time import sleep, time
from geometry_msgs.msg import *
from move_base_msgs.msg import *
from sensor_msgs.msg import Image
from std_msgs.msg import Bool, Int32
from actionlib_msgs.msg import GoalID
from visualization_msgs.msg import MarkerArray
from jetarm.Comm import ftservo
import serial
from jetarm import arm_v1servo

ser = serial.Serial(port="/dev/XCOM1", baudrate=1000000)
#ser=serial.Serial(port="/dev/XCOM1",baudrate=1000000)


class ROSNav:
    def __init__(self):
        rospy.on_shutdown(self.cancel)
        self.InitialParam()
        self.pub_CmdVel = rospy.Publisher('/cmd_vel', Twist, queue_size=10)
        self.pub_goal = rospy.Publisher('move_base_simple/goal', PoseStamped, queue_size=1)
        self.pub_cancel = rospy.Publisher("move_base/cancel", GoalID, queue_size=10)
        self.sub_markerArray = rospy.Subscriber('color_end_pose', MarkerArray, self.getMarker_callback)
        self.sub_goal_result = rospy.Subscriber('move_base/result', MoveBaseActionResult, self.goal_result_callback)



    def InitialParam(self):
        pst = PoseStamped()
        pst.pose.orientation.w = 1
        self.start_point = pst.pose
        self.goal_result = 0
        self.Transport_status = False
        #self.grip_down =False
        self.markerArray = MarkerArray()
        self.color_pose={}


    def PubTargetPoint(self, goal_pose):
        self.Transport_status = True
        pose = PoseStamped()
        pose.header.frame_id = 'map'
        pose.header.stamp = rospy.Time.now()
        pose.pose = goal_pose
        self.pub_goal.publish(pose)

    def getMarker_callback(self, msg):
        if not isinstance(msg, MarkerArray): return
        self.color_pose={}
        for i in msg.markers:
            if i.id == 0: self.color_pose[0] = i.pose
            if i.id == 5: self.start_point = i.pose



    def goal_result_callback(self, msg):
        if not isinstance(msg, MoveBaseActionResult): return
        if msg.status.SUCCEEDED == 3:self.Transport_status = False
        self.goal_result = msg.status.SUCCEEDED

    def pubVel(self, x, y, z=0):
        twist = Twist()
        twist.linear.x = x
        twist.linear.y = y
        twist.angular.z = z
        self.pub_CmdVel.publish(twist)
        self.RobotRun_status = False

    def cancel(self):
        self.pub_cancel.publish(GoalID())
        self.pubVel(0, 0, 0)
        self.pub_goal.unregister()
        self.pub_CmdVel.unregister()
        self.pub_cancel.unregister()
        self.sub_goal_result.unregister()
        self.sub_markerArray.unregister()

class ColorTransport:
    def __init__(self):
        self.ros_nav = ROSNav()
        self.model = "Init"
        self.i=0
        self.arm = arm_v1servo.Arm_v1servo(ser, lower_arm_servo_reset_angle=0)
        self.X=0
        self.Z=0
        self.rate=rospy.Rate(10)
        self.vel_pub = rospy.Publisher('cmd_vel', Twist, queue_size=10)
        self.Pose_sub = rospy.Subscriber("object_detect_pose", Pose, self.poseCallback)
        vel=Twist()
            
    def poseCallback(self,Pose):

        self.X = Pose.position.x;
        self.Z = Pose.position.z;
        
    def handle(self):
        
        global vel
        if self.X > 340 and self.X < 350 :
            vel = Twist()
        elif self.X < 340 :
            vel = Twist()
            vel.angular.z = 0.1
        elif self.X > 350 :
            vel = Twist()
            vel.angular.z = -0.1
        else:
            print("No X,cannot control!")
        self.vel_pub.publish(vel)
        
        
        if self.Z >=2385 and self.Z <= 2405 :                            
            vel = Twist()
        elif self.Z < 2385 :
            vel = Twist()
            vel.linear.x = 0.1
        #elif self.z> 2405:
            #vel = Twist()
            #vel.linear.x = -0.1
        else:
            print("No Z,cannot control!")
        self.vel_pub.publish(vel)
          
    def process(self):
             
        self.b=input()

        if self.b.isalpha():
            self.model="Transport"
        if self.b.isdigit():
            self.model="come_back"
            
        if self.model == "Transport":
            self.arm.angle_control(-65,50,-50,24,50,200)
            sleep(6)
            start_time=rospy.Time.now() 
            while (rospy.Time.now()-start_time)<rospy.Duration(10):
                self.handle()
                self.rate.sleep()
            sleep(5)
            self.arm.angle_control(0, -5, -45, 24, 50, 200)
            sleep(6)
            self.arm.angle_control(0, 55, -60, 90, 50, 250)
            sleep(6)
            self.arm.angle_control(3, 51, -43, 90, 50, 250)
            sleep(5)
            self.arm.angle_control(3, 51, -43, 24, 50, 250)
            sleep(10)
            self.arm.angle_control(22, -5, -45, 24, 50, 200)
            sleep(10)
            
            if not self.ros_nav.Transport_status:
                self.ros_nav.PubTargetPoint(self.ros_nav.color_pose[0])
                self.ros_nav.Transport_status = True
              
        
        if self.model == "come_back":

            if self.i == 0:
                self.arm.angle_control(0, 63, -35, 24, 50, 150)
                sleep(10)
                self.arm.angle_control(0, 63, -35, 90, 50, 150)
                sleep(10)
                self.arm.angle_control(0, 30, -58, 90, 50, 150)
                sleep(10)
                self.arm.angle_control(22, -5, -45, 90, 50, 150)
            elif self.i == 1:
                self.arm.angle_control(-65,50,-50,24,50,150)
                sleep(10)
                start_time=rospy.Time.now() 
                while (rospy.Time.now()-start_time)<rospy.Duration(10):
                    self.handle()
                    self.rate.sleep()
                self.arm.angle_control(3, 30, -40, 24, 50, 250)
                sleep(10)
                self.arm.angle_control(3, 27, -45, 90, 50, 250)
                sleep(10)
                self.arm.angle_control(3, 10, -45, 90, 50, 150)
                sleep(10)
                self.arm.angle_control(22, -5, -45, 90, 50, 150)

            elif self.i ==2:              
                self.arm.angle_control(18, 54, -55, 24, 50, 100)
                sleep(10)
                self.arm.angle_control(18, 30, -45, 90, 50, 100)
                sleep(10)
                self.arm.angle_control(22, -5, -45, 90, 50, 100)
            else:
                self.arm.angle_control(-65,50,-50,24,50,150)
                sleep(10)
                start_time=rospy.Time.now() 
                while (rospy.Time.now()-start_time)<rospy.Duration(10):
                    self.handle()
                    self.rate.sleep()
                self.arm.angle_control(3, 30, -40, 24, 50, 250)
                sleep(10)
                self.arm.angle_control(3, 27, -45, 90, 50, 250)
                sleep(10)
                self.arm.angle_control(3, 10, -45, 90, 50, 150)
                sleep(10)
                self.arm.angle_control(22, -5, -45, 90, 50, 150)

            sleep(10)    
            self.comeback()
            if self.ros_nav.goal_result == 3:
                self.Reset()
            if self.i <4:
                self.i+=1

    


    def comeback(self):
        self.ros_nav.PubTargetPoint(self.ros_nav.start_point)


    def Reset(self):
        self.ros_nav.goal_result = 0
        self.model = "Init"
        self.ros_nav.Transport_status = False


if __name__ == '__main__':
    rospy.init_node('color_transport',anonymous=False)
    color_transport = ColorTransport()

    rate=rospy.Rate(10)

    while not rospy.is_shutdown():
         color_transport.process()
         #color_transport.handle()
         rate.sleep()
    try:
        rospy.spin()
    except rospy.ROSInterruptException:
        print('over')


