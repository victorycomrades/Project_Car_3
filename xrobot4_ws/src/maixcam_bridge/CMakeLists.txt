cmake_minimum_required(VERSION 3.0.2)
project(maixcam_bridge)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  rospy
  std_msgs
)

catkin_python_setup()

catkin_package(
  CATKIN_DEPENDS geometry_msgs rospy std_msgs
)

catkin_install_python(PROGRAMS
  scripts/maixcam_bridge_node.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

