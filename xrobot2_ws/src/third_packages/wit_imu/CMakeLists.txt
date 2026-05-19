cmake_minimum_required(VERSION 3.7.2)
project(wit_imu)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
)

catkin_package()

catkin_python_setup()

include_directories(
  ${catkin_INCLUDE_DIRS}
)

catkin_install_python(
  PROGRAMS scripts/wit_normal_ros.py scripts/wit_modbus_ros.py scripts/get_imu_rpy.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)