################################################################################
## Set minimum required version of cmake, project name and compile options    ##
################################################################################
cmake_minimum_required(VERSION 3.0.2)
project(xrobot_driver)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  dynamic_reconfigure
  sensor_msgs
  diagnostic_msgs
  xrobot_msgs
)

catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################


################################################
## Declare ROS dynamic reconfigure parameters ##
################################################
generate_dynamic_reconfigure_options(
  cfg/ChassisParams.cfg
)


###################################
## catkin specific configuration ##
###################################
catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES xrobot_driver
 CATKIN_DEPENDS roscpp rospy dynamic_reconfigure sensor_msgs diagnostic_msgs xrobot_msgs
#  DEPENDS system_lib
)

###########
## Build ##
###########
include_directories(
#  include
 ${catkin_INCLUDE_DIRS}
)

#############
## Install ##
#############
catkin_install_python(PROGRAMS
  scripts/serial_node.py
  scripts/xrobot_diagnostics.py
  scripts/xrobot_servo.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)


#############
## Testing ##
#############

