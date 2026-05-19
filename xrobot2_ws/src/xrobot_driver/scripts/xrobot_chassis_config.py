#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from dynamic_reconfigure.server import Server
from xrobot_msgs.msg import ParamConfig
from xrobot_driver.cfg import ChassisParamsConfig

from xrobot_tools import *


class ChassisParams():
    def __init__(self):
        self.save_params_ = False
        self.chassis_param_pub_ = rospy.Publisher('chassis_params', ParamConfig, queue_size = 1)
        srv_ = Server(ChassisParamsConfig, self.param_cb)
        

    def param_cb(self, config, level):
        if config.save_params and not self.save_params_:
            rospy.loginfo("Save params: chassis_type : %f, x_speed_scale : %f, y_speed_scale : %f, w_speed_scale : %f", 
                        config.chassis_type, config.x_speed_scale, config.y_speed_scale, config.w_speed_scale)

            param = ParamConfig(chassis_type = config.chassis_type,
                                x_speed_scale = config.x_speed_scale,
                                y_speed_scale = config.y_speed_scale,
                                w_speed_scale = config.w_speed_scale)

            self.chassis_param_pub_.publish(param)
        self.save_params_ = config.save_params
        return config
    
    def main(self):
        rospy.spin()

if __name__ == '__main__':
    rospy.init_node('chassis_params')
    node = ChassisParams()
    node.main()
