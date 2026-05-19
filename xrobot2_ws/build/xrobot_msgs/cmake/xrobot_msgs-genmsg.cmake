# generated from genmsg/cmake/pkg-genmsg.cmake.em

message(STATUS "xrobot_msgs: 3 messages, 0 services")

set(MSG_I_FLAGS "-Ixrobot_msgs:/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg;-Istd_msgs:/opt/ros/noetic/share/std_msgs/cmake/../msg")

# Find all generators
find_package(gencpp REQUIRED)
find_package(geneus REQUIRED)
find_package(genlisp REQUIRED)
find_package(gennodejs REQUIRED)
find_package(genpy REQUIRED)

add_custom_target(xrobot_msgs_generate_messages ALL)

# verify that message/service dependencies have not changed since configure



get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" NAME_WE)
add_custom_target(_xrobot_msgs_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "xrobot_msgs" "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" ""
)

get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" NAME_WE)
add_custom_target(_xrobot_msgs_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "xrobot_msgs" "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" ""
)

get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" NAME_WE)
add_custom_target(_xrobot_msgs_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "xrobot_msgs" "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" "std_msgs/Header"
)

#
#  langs = gencpp;geneus;genlisp;gennodejs;genpy
#

### Section generating for lang: gencpp
### Generating Messages
_generate_msg_cpp(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_cpp(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_cpp(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/xrobot_msgs
)

### Generating Services

### Generating Module File
_generate_module_cpp(xrobot_msgs
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/xrobot_msgs
  "${ALL_GEN_OUTPUT_FILES_cpp}"
)

add_custom_target(xrobot_msgs_generate_messages_cpp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_cpp}
)
add_dependencies(xrobot_msgs_generate_messages xrobot_msgs_generate_messages_cpp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_cpp _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_cpp _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_cpp _xrobot_msgs_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(xrobot_msgs_gencpp)
add_dependencies(xrobot_msgs_gencpp xrobot_msgs_generate_messages_cpp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS xrobot_msgs_generate_messages_cpp)

### Section generating for lang: geneus
### Generating Messages
_generate_msg_eus(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_eus(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_eus(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/xrobot_msgs
)

### Generating Services

### Generating Module File
_generate_module_eus(xrobot_msgs
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/xrobot_msgs
  "${ALL_GEN_OUTPUT_FILES_eus}"
)

add_custom_target(xrobot_msgs_generate_messages_eus
  DEPENDS ${ALL_GEN_OUTPUT_FILES_eus}
)
add_dependencies(xrobot_msgs_generate_messages xrobot_msgs_generate_messages_eus)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_eus _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_eus _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_eus _xrobot_msgs_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(xrobot_msgs_geneus)
add_dependencies(xrobot_msgs_geneus xrobot_msgs_generate_messages_eus)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS xrobot_msgs_generate_messages_eus)

### Section generating for lang: genlisp
### Generating Messages
_generate_msg_lisp(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_lisp(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_lisp(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/xrobot_msgs
)

### Generating Services

### Generating Module File
_generate_module_lisp(xrobot_msgs
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/xrobot_msgs
  "${ALL_GEN_OUTPUT_FILES_lisp}"
)

add_custom_target(xrobot_msgs_generate_messages_lisp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_lisp}
)
add_dependencies(xrobot_msgs_generate_messages xrobot_msgs_generate_messages_lisp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_lisp _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_lisp _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_lisp _xrobot_msgs_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(xrobot_msgs_genlisp)
add_dependencies(xrobot_msgs_genlisp xrobot_msgs_generate_messages_lisp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS xrobot_msgs_generate_messages_lisp)

### Section generating for lang: gennodejs
### Generating Messages
_generate_msg_nodejs(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_nodejs(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_nodejs(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/xrobot_msgs
)

### Generating Services

### Generating Module File
_generate_module_nodejs(xrobot_msgs
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/xrobot_msgs
  "${ALL_GEN_OUTPUT_FILES_nodejs}"
)

add_custom_target(xrobot_msgs_generate_messages_nodejs
  DEPENDS ${ALL_GEN_OUTPUT_FILES_nodejs}
)
add_dependencies(xrobot_msgs_generate_messages xrobot_msgs_generate_messages_nodejs)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_nodejs _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_nodejs _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_nodejs _xrobot_msgs_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(xrobot_msgs_gennodejs)
add_dependencies(xrobot_msgs_gennodejs xrobot_msgs_generate_messages_nodejs)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS xrobot_msgs_generate_messages_nodejs)

### Section generating for lang: genpy
### Generating Messages
_generate_msg_py(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_py(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs
)
_generate_msg_py(xrobot_msgs
  "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg"
  "${MSG_I_FLAGS}"
  "/opt/ros/noetic/share/std_msgs/cmake/../msg/Header.msg"
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs
)

### Generating Services

### Generating Module File
_generate_module_py(xrobot_msgs
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs
  "${ALL_GEN_OUTPUT_FILES_py}"
)

add_custom_target(xrobot_msgs_generate_messages_py
  DEPENDS ${ALL_GEN_OUTPUT_FILES_py}
)
add_dependencies(xrobot_msgs_generate_messages xrobot_msgs_generate_messages_py)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/ParamConfig.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_py _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/VersionInfo.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_py _xrobot_msgs_generate_messages_check_deps_${_filename})
get_filename_component(_filename "/home/xrobot/xrobot2_ws/src/xrobot_msgs/msg/SensorState.msg" NAME_WE)
add_dependencies(xrobot_msgs_generate_messages_py _xrobot_msgs_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(xrobot_msgs_genpy)
add_dependencies(xrobot_msgs_genpy xrobot_msgs_generate_messages_py)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS xrobot_msgs_generate_messages_py)



if(gencpp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/xrobot_msgs)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/xrobot_msgs
    DESTINATION ${gencpp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_cpp)
  add_dependencies(xrobot_msgs_generate_messages_cpp std_msgs_generate_messages_cpp)
endif()

if(geneus_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/xrobot_msgs)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/xrobot_msgs
    DESTINATION ${geneus_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_eus)
  add_dependencies(xrobot_msgs_generate_messages_eus std_msgs_generate_messages_eus)
endif()

if(genlisp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/xrobot_msgs)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/xrobot_msgs
    DESTINATION ${genlisp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_lisp)
  add_dependencies(xrobot_msgs_generate_messages_lisp std_msgs_generate_messages_lisp)
endif()

if(gennodejs_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/xrobot_msgs)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/xrobot_msgs
    DESTINATION ${gennodejs_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_nodejs)
  add_dependencies(xrobot_msgs_generate_messages_nodejs std_msgs_generate_messages_nodejs)
endif()

if(genpy_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs)
  install(CODE "execute_process(COMMAND \"/usr/bin/python3\" -m compileall \"${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs\")")
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/xrobot_msgs
    DESTINATION ${genpy_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_py)
  add_dependencies(xrobot_msgs_generate_messages_py std_msgs_generate_messages_py)
endif()
