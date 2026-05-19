cmake_minimum_required(VERSION 3.0.2)
project(xrobot_description)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
find_package(catkin REQUIRED COMPONENTS
  urdf
  xacro
)

################################################
## Declare ROS messages, services and actions ##
################################################


################################################
## Declare ROS dynamic reconfigure parameters ##
################################################


###################################
## catkin specific configuration ##
###################################
catkin_package(
  CATKIN_DEPENDS urdf xacro
)

###########
## Build ##
###########
include_directories(
  ${catkin_INCLUDE_DIRS}
)


#############
## Install ##
#############
install(DIRECTORY meshes rviz urdf
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

#############
## Testing ##
#############
