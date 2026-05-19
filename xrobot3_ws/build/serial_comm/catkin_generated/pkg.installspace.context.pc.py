# generated from catkin/cmake/template/pkg.context.pc.in
CATKIN_PACKAGE_PREFIX = ""
PROJECT_PKG_CONFIG_INCLUDE_DIRS = "${prefix}/include".split(';') if "${prefix}/include" != "" else []
PROJECT_CATKIN_DEPENDS = "roscpp;std_msgs;sensor_msgs;geometry_msgs;tf;message_runtime".replace(';', ' ')
PKG_CONFIG_LIBRARIES_WITH_PREFIX = "-lserial_comm".split(';') if "-lserial_comm" != "" else []
PROJECT_NAME = "serial_comm"
PROJECT_SPACE_DIR = "/home/xrobot/xrobot3_ws/install"
PROJECT_VERSION = "0.0.1"
