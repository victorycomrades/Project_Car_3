#!/bin/bash

# 启动 ROS 工作流
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# 这里需要根据你 Jetson Nano 的 ROS 版本修改，例如 noetic
source /opt/ros/noetic/setup.bash

if [ -f "$PROJECT_ROOT/devel/setup.bash" ]; then
  source "$PROJECT_ROOT/devel/setup.bash"
fi

export ROS_MASTER_URI=http://localhost:11311
export ROS_HOSTNAME=localhost

cd "$PROJECT_ROOT"
exec roslaunch xrobot_arm task_flow.launch
