cmake_minimum_required(VERSION 2.8.3)
project(robot_bringup)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  serial_comm
  robot_ctrl
)
catkin_package(
  CATKIN_DEPENDS roscpp serial_comm robot_ctrl
)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

install(
  DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)