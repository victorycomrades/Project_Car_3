cmake_minimum_required(VERSION 3.0.2)
project(maixcam_uart_bridge)

find_package(catkin REQUIRED COMPONENTS
  rospy
  std_msgs
  geometry_msgs
  sensor_msgs
)

catkin_package()

catkin_install_python(
  PROGRAMS
    scripts/maixcam_uart_bridge_node.py
    scripts/arm_servo_angle_reader_node.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(
  DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)

install(
  DIRECTORY src/maixcam_uart_bridge/
  DESTINATION ${CATKIN_PACKAGE_PYTHON_DESTINATION}
)

install(
  DIRECTORY test/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/test
)
