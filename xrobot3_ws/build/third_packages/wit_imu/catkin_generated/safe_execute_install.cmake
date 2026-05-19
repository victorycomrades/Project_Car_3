execute_process(COMMAND "/home/xrobot/xrobot3_ws/build/third_packages/wit_imu/catkin_generated/python_distutils_install.sh" RESULT_VARIABLE res)

if(NOT res EQUAL 0)
  message(FATAL_ERROR "execute_process(/home/xrobot/xrobot3_ws/build/third_packages/wit_imu/catkin_generated/python_distutils_install.sh) returned error code ")
endif()
