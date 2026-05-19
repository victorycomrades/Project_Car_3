#!/usr/bin/env python3

THIS_PACKAGE = "xrobot_driver"

__usage__ = """
make_libraries.py generates the XRobot rosserial library files.  It
requires the location of your XRobot sketchbook/libraries folder.

rosrun xrobot_driver make_libraries.py <output_path>
"""

import rospkg
import rosserial_client
from rosserial_client.make_library import *

# for copying files
import shutil
import os.path

ROS_TO_EMBEDDED_TYPES = {
    'bool'    :   ('bool',              1, PrimitiveDataType, []),
    'byte'    :   ('int8_t',            1, PrimitiveDataType, []),
    'int8'    :   ('int8_t',            1, PrimitiveDataType, []),
    'char'    :   ('uint8_t',           1, PrimitiveDataType, []),
    'uint8'   :   ('uint8_t',           1, PrimitiveDataType, []),
    'int16'   :   ('int16_t',           2, PrimitiveDataType, []),
    'uint16'  :   ('uint16_t',          2, PrimitiveDataType, []),
    'int32'   :   ('int32_t',           4, PrimitiveDataType, []),
    'uint32'  :   ('uint32_t',          4, PrimitiveDataType, []),
    'int64'   :   ('int64_t',           8, PrimitiveDataType, []),
    'uint64'  :   ('uint64_t',          8, PrimitiveDataType, []),
    'float32' :   ('float',             4, PrimitiveDataType, []),
    'float64' :   ('float',             4, AVR_Float64DataType, []),
    'time'    :   ('ros::Time',         8, TimeDataType, ['ros/time']),
    'duration':   ('ros::Duration',     8, TimeDataType, ['ros/duration']),
    'string'  :   ('char*',             0, StringDataType, []),
    'Header'  :   ('std_msgs::Header',  0, MessageDataType, ['std_msgs/Header'])
}

# need correct inputs
if (len(sys.argv) < 2):
    print(__usage__)
    exit()

# get output path
path = sys.argv[1]
output_path = os.path.join(sys.argv[1], "ros_lib")
print("\nExporting to %s" % output_path)

rospack = rospkg.RosPack()

# copy ros_lib stuff in
# shutil.rmtree(output_path, ignore_errors=True)
# shutil.copytree(os.path.join(rospack.get_path(THIS_PACKAGE), "src", "ros_lib"), output_path)
# rosserial_client_copy_files(rospack, output_path)

# generate messages
rosserial_generate(rospack, output_path, ROS_TO_EMBEDDED_TYPES)
