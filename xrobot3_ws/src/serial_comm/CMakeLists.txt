cmake_minimum_required(VERSION 2.8.3)
project(serial_comm)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  sensor_msgs
  geometry_msgs
  tf
  message_generation
)

add_message_files(
  FILES
  RobotControl.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp std_msgs sensor_msgs geometry_msgs tf message_runtime
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
  src/crc16.cpp
  src/serial_comm.cpp
)

target_link_libraries(${PROJECT_NAME}
  ${catkin_LIBRARIES}
  -lserial
)

add_executable(serial_node src/serial_node.cpp)

target_link_libraries(serial_node
  ${PROJECT_NAME}
  ${catkin_LIBRARIES}
)

add_dependencies(serial_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})