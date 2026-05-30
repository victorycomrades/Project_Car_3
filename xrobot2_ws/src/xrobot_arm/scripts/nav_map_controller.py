#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
import actionlib
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
from geometry_msgs.msg import PoseStamped
from std_msgs.msg import Bool

class NavMapController:
    def __init__(self):
        self.client = actionlib.SimpleActionClient('move_base', MoveBaseAction)
        self.client.wait_for_server()
        self.goal_sub = rospy.Subscriber('/nav_goal', PoseStamped, self.goal_callback)
        self.cancel_sub = rospy.Subscriber('/nav_cancel', Bool, self.cancel_callback)
        self.status_pub = rospy.Publisher('/nav_status', Bool, queue_size=10)
        rospy.loginfo("NavMapController initialized")

    def goal_callback(self, msg):
        goal = MoveBaseGoal()
        goal.target_pose = msg
        goal.target_pose.header.frame_id = 'map'  # 假设目标在地图框架中
        goal.target_pose.header.stamp = rospy.Time.now()
        self.client.send_goal(goal)
        self.client.wait_for_result()
        if self.client.get_state() == actionlib.GoalStatus.SUCCEEDED:
            self.status_pub.publish(True)
            rospy.loginfo("Navigation succeeded")
        else:
            self.status_pub.publish(False)
            rospy.logwarn("Navigation failed")

    def cancel_callback(self, msg):
        if msg.data:
            self.client.cancel_all_goals()
            rospy.loginfo("Navigation cancelled")

if __name__ == '__main__':
    rospy.init_node('nav_map_controller')
    controller = NavMapController()
    rospy.spin()