cmake_minimum_required(VERSION 2.8.3)
project(robot_ctrl)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  sensor_msgs
  geometry_msgs
  serial_comm
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}_pid
  CATKIN_DEPENDS roscpp std_msgs sensor_msgs geometry_msgs serial_comm
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}_pid
  src/Algorithm/pid.cpp
)

target_link_libraries(${PROJECT_NAME}_pid
  ${catkin_LIBRARIES}
)

add_executable(robot_ctrl_node src/Robot_CTRL.cpp)

target_link_libraries(robot_ctrl_node
  ${PROJECT_NAME}_pid
  ${catkin_LIBRARIES}
)

add_dependencies(robot_ctrl_node ${catkin_EXPORTED_TARGETS})