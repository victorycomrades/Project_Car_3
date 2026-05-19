#!/usr/bin/env python3
# encoding: utf-8
import rospy
import threading
from time import sleep
from geometry_msgs.msg import *
from move_base_msgs.msg import *
from sensor_msgs.msg import Image
from std_msgs.msg import Bool, Int32
from actionlib_msgs.msg import GoalID
from visualization_msgs.msg import MarkerArray

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
        for i in msg.markers:
            if i.id == 1: self.color_pose[1] = i.pose
            if i.id == 2: self.start_point = i.pose

        self.color_pose=msg.pose

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


