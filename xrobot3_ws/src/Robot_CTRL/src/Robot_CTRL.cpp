/**
 *****************************************************************
 * @file Robot_CTRL.cpp
 * @brief 底盘控制ROS1节点
 * 
 * 实现底盘控制的ROS1节点，订阅电机状态、IMU数据和手柄输入，发布电机速度命令
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */

#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Joy.h>
#include <std_msgs/Int32MultiArray.h>
#include <cmath>
#include "Algorithm/pid.hpp"
#include "serial_comm/RobotControl.h"

// PID控制器
PidTypeDef vw_pid_;  ///< 角速度PID控制器

// 控制模式枚举
enum ChassisCtrlMode_e
{
    CHASSIS_RELAX = 0,      // 放松模式
    CHASSIS_STOP,           // 停止模式
    CHASSIS_NORMAL,         // 正常模式
};

// 底盘机械结构参数结构体
typedef struct
{
    double wheel_perimeter; /* 轮子周长(mm) */
    double wheeltrack;      /* 轮距(mm) */
    double wheelbase;       /* 轴距(mm) */
    double rotate_x_offset; /* 相对于底盘中心的x轴旋转偏移(mm) */
    double rotate_y_offset; /* 相对于底盘中心的y轴旋转偏移(mm) */
    double radius;          /* 底盘半径(mm) */
} MechanicalStructure_t;

// 底盘运动控制结构体
typedef struct
{
    double vx;                 // 线速度x方向
    double vy;                 // 线速度y方向
    double vw;                 // 角速度
    double wheel_rpm[4];       // 轮子转速(RPM)
} Chassis_MotionControl;

/**
 * @brief 底盘控制节点类
 * 
 * 负责订阅电机状态、IMU数据和手柄输入，实现底盘运动控制算法
 */
class ChassisCtrlNode
{
public:
    /**
     * @brief 构造函数
     */
    ChassisCtrlNode()
    {
        ros::NodeHandle nh("~");
        
        // 获取参数
        nh.param("linear_scale", linear_scale_, 500.0);
        nh.param("angular_scale", angular_scale_, 100.0);
        nh.param("max_speed", max_speed_, 1000);  // 最大速度限制（RPM）
        
        // 创建订阅者
        motor_states_subscriber_ = nh.subscribe("/motor_states", 10, &ChassisCtrlNode::motor_states_callback, this);
        
        imu_subscriber_ = nh.subscribe("/imu/data", 10, &ChassisCtrlNode::imu_callback, this);
        
        joy_subscriber_ = nh.subscribe("/joy", 10, &ChassisCtrlNode::joy_callback, this);
        
        // 创建发布者 - 修改话题为/robot_cmd，与串口节点匹配
        motor_control_publisher_ = nh.advertise<serial_comm::RobotControl>("/robot_cmd", 10);
        
        // 初始化控制模式
        ctrl_mode_ = CHASSIS_RELAX;
        last_ctrl_mode_ = CHASSIS_RELAX;
        
        // 初始化底盘参数（从robot_def.h获取）
        structure_.wheel_perimeter = 376.99;  // WHEEL_PERIMETER
        structure_.wheeltrack = 351.0;        // WHEELTRACK
        structure_.wheelbase = 351.0;         // WHEELBASE
        structure_.radius = 248.195;          // RADIUS
        structure_.rotate_x_offset = 0.0;     // GIMBAL_X_OFFSET
        structure_.rotate_y_offset = 0.0;     // GIMBAL_Y_OFFSET
        
        // 初始化电机速度指令
        for (int i = 0; i < 4; i++) {
            motor_speed_commands_[i] = 0;
            motion_ctrl_.wheel_rpm[i] = 0.0;
        }
        
        // 初始化速度指令
        vx_ = 0.0;
        vy_ = 0.0;
        vw_ = 0.0;
        
        // 初始化角速度PID控制器
        const float vw_pid_params[3] = {5.0f, 0.0f, 0.0f};  // Kp, Ki, Kd
        PID_init(&vw_pid_, PID_MODE::PID_POSITION, vw_pid_params, 100.0f, 10.0f);

        ROS_INFO("底盘控制节点初始化成功");
        ROS_INFO("线性速度缩放因子: %.2f", linear_scale_);
        ROS_INFO("角速度缩放因子: %.2f", angular_scale_);
        ROS_INFO("最大速度限制: %d RPM", max_speed_);
    }
    
    /**
     * @brief 析构函数
     */
    ~ChassisCtrlNode()
    {
        // 发送零速度命令
        send_zero_speed();
        
        ROS_INFO("底盘控制节点已关闭");
    }
    
    /**
     * @brief 运行节点
     */
    void spin()
    {
        ros::Rate rate(100); // 100Hz
        while (ros::ok()) {
            control_loop();
            ros::spinOnce();
            rate.sleep();
        }
    }
    
private:
    /**
     * @brief 电机状态回调函数
     * @param msg 电机状态消息
     */
    void motor_states_callback(const sensor_msgs::JointState::ConstPtr& msg)
    {
        // 保存电机状态
        if (msg->name.size() >= 4 && msg->position.size() >= 4 && msg->velocity.size() >= 4) {
            for (int i = 0; i < 4; i++) {
                motor_positions_[i] = msg->position[i];
                motor_velocities_[i] = msg->velocity[i];
            }
        }
    }
    
    /**
     * @brief IMU数据回调函数
     * @param msg IMU数据消息
     */
    void imu_callback(const sensor_msgs::Imu::ConstPtr& msg)
    {
        // 保存IMU数据
        imu_orientation_ = msg->orientation;
        imu_angular_velocity_ = msg->angular_velocity;
        imu_linear_acceleration_ = msg->linear_acceleration;
    }
    
    /**
     * @brief 手柄输入回调函数
     * @param msg 手柄输入消息
     */
    void joy_callback(const sensor_msgs::Joy::ConstPtr& msg)
    {
        // 读取手柄输入
        // 假设使用左摇杆控制前进/后退/左右平移，右摇杆控制旋转
        // 具体按钮映射根据实际手柄调整
        
        if (msg->axes.size() >= 4) {
            // 获取速度指令
            vx_ = msg->axes[1] * -linear_scale_;  // 前进/后退
            vy_ = msg->axes[0] * -linear_scale_;  // 左右平移
            vw_ = msg->axes[2] * angular_scale_;  // 旋转
        }
        
        // 读取按钮输入（可选）
        if (msg->buttons.size() > 0) {
            if (msg->buttons[0]) {  // 按下A键切换到放松模式
                ctrl_mode_ = CHASSIS_RELAX;
                ROS_INFO("切换到放松模式");
            } else if (msg->buttons[1]) {  // 按下B键切换到停止模式
                ctrl_mode_ = CHASSIS_STOP;
                ROS_INFO("切换到停止模式");
            } else if (msg->buttons[3]) {  // 按下X键切换到正常模式
                ctrl_mode_ = CHASSIS_NORMAL;
                ROS_INFO("切换到正常模式");
            }
        }
    }
    
    /**
     * @brief 控制循环函数
     */
    void control_loop()
    {
        // 控制模式切换
        chassis_ctrl_mode_switch();
        
        // 根据控制模式执行不同的控制逻辑
        switch (ctrl_mode_)
        {
            case CHASSIS_NORMAL:
            {
                chassis_normal_mode();
                break;
            }
            case CHASSIS_STOP:
            {
                chassis_stop_mode();
                break;
            }
            default:
                break;
        }
        
        // 底盘控制解算
        chassis_control_calc_loop();
        
        // 将计算得到的速度指令转换为整数并限制范围
        for (int i = 0; i < 4; i++) {
            motor_speed_commands_[i] = static_cast<int16_t>(motion_ctrl_.wheel_rpm[i]);
            
            // 限制速度范围
            if (motor_speed_commands_[i] > max_speed_) {
                motor_speed_commands_[i] = max_speed_;
            } else if (motor_speed_commands_[i] < -max_speed_) {
                motor_speed_commands_[i] = -max_speed_;
            }
        }
        
        // 如果是放松模式，发送零速度
        if (ctrl_mode_ == CHASSIS_RELAX) {
            for (int i = 0; i < 4; i++) {
                motor_speed_commands_[i] = 0;
            }
        }
        
        // 发送电机速度控制命令
        send_motor_speed_command();
    }
    
    /**
     * @brief 控制模式切换函数
     */
    void chassis_ctrl_mode_switch()
    {
        // 这里可以根据实际需求添加更多的控制模式切换逻辑
        // 例如：根据遥控器输入、按钮状态等
        
        // 如果控制模式发生变化，记录并输出日志
        if (ctrl_mode_ != last_ctrl_mode_) {
            ROS_INFO("控制模式切换: %d -> %d", last_ctrl_mode_, ctrl_mode_);
            last_ctrl_mode_ = ctrl_mode_;
        }
    }
    
    /**
     * @brief 正常模式控制逻辑
     */
    void chassis_normal_mode()
    {
        // 根据手柄输入设置速度指令
        // 这里可以添加更多的控制逻辑，例如：跟随云台、速度限制等
        
        // 示例：直接使用手柄输入的速度指令
        motion_ctrl_.vx = vx_;
        motion_ctrl_.vy = vy_;
        motion_ctrl_.vw = vw_;

        // 角速度PID控制
        // 目标角速度：遥控器输入的vw_
        // 反馈量：陀螺仪的z轴角速度
        //float gyro_z_velocity = imu_angular_velocity_.z;
        //motion_ctrl_.vw = PID_Calc(&vw_pid_, gyro_z_velocity, vw_);

        //ROS_INFO("速度: %.2f, %.2f, %.2f", motion_ctrl_.vx, motion_ctrl_.vy, motion_ctrl_.vw);

    }
    
    /**
     * @brief 停止模式控制逻辑
     */
    void chassis_stop_mode()
    {
        // 设置所有速度指令为0
        motion_ctrl_.vx = 0.0;
        motion_ctrl_.vy = 0.0;
        motion_ctrl_.vw = 0.0;
    }
    
    /**
     * @brief 底盘运动转换
     * @param chassis_vx 转换后的x方向速度
     * @param chassis_vy 转换后的y方向速度
     */
    void chassis_move_transform(double* chassis_vx, double* chassis_vy)
    {
        // 这里可以添加底盘运动转换逻辑，例如：根据云台角度调整运动方向
        // 当前简单实现：直接使用输入速度
        *chassis_vx = motion_ctrl_.vx ;
        *chassis_vy = motion_ctrl_.vy;
    }
    
    /**
     * @brief 麦轮底盘运动解算
     * @param chassis_vx x方向速度
     * @param chassis_vy y方向速度
     * @param chassis_vw 角速度
     */
    void omni_calculate(double chassis_vx, double chassis_vy, double chassis_vw)
    {
        // 计算旋转比例
        double rotate_ratio_fr = ((structure_.wheelbase + structure_.wheeltrack) / 2.0 - 
                               structure_.rotate_x_offset + structure_.rotate_y_offset) / RADIAN_COEF;
        double rotate_ratio_fl = ((structure_.wheelbase + structure_.wheeltrack) / 2.0 - 
                               structure_.rotate_x_offset - structure_.rotate_y_offset) / RADIAN_COEF;
        double rotate_ratio_bl = ((structure_.wheelbase + structure_.wheeltrack) / 2.0 + 
                               structure_.rotate_x_offset - structure_.rotate_y_offset) / RADIAN_COEF;
        double rotate_ratio_br = ((structure_.wheelbase + structure_.wheeltrack) / 2.0 + 
                               structure_.rotate_x_offset + structure_.rotate_y_offset) / RADIAN_COEF;
        
        // 计算轮子转速比例（将mm/s转换为RPM）
        // M3508_REDUCTION_RATIO = 1 / 19.0
        double wheel_rpm_ratio = (structure_.wheel_perimeter / 19.0) / 60.0;
        
        // 计算每个轮子的转速
        double wheel_rpm[4];
        wheel_rpm[0] = (-chassis_vx - chassis_vy - chassis_vw * rotate_ratio_bl) * wheel_rpm_ratio;
        wheel_rpm[1] = (chassis_vx - chassis_vy - chassis_vw * rotate_ratio_fr) * wheel_rpm_ratio;

        wheel_rpm[2] =(chassis_vx + chassis_vy - chassis_vw * rotate_ratio_fl) * wheel_rpm_ratio;
        wheel_rpm[3] =(-chassis_vx + chassis_vy - chassis_vw * rotate_ratio_br) * wheel_rpm_ratio;
        
        // 保存计算结果
        for (int i = 0; i < 4; i++) {
            motion_ctrl_.wheel_rpm[i] = wheel_rpm[i];
        }

        ROS_INFO("4_moto_speed: %.2f, %.2f, %.2f, %.2f",wheel_rpm[0], wheel_rpm[1], wheel_rpm[2], wheel_rpm[3]);

    }
    
    /**
     * @brief 底盘控制解算循环
     */
    void chassis_control_calc_loop()
    {
        double chassis_vx = 0.0, chassis_vy = 0.0;
        
        // 底盘运动转换
        chassis_move_transform(&chassis_vx, &chassis_vy);
        
        // 麦轮底盘运动解算
        omni_calculate(chassis_vx, chassis_vy, motion_ctrl_.vw);
    }
    
    /**
     * @brief 发送电机速度控制命令
     */
    void send_motor_speed_command()
    {
        serial_comm::RobotControl msg;
        
        // 设置电机速度
        msg.motor1_speed = motor_speed_commands_[0];
        msg.motor2_speed = motor_speed_commands_[1];
        msg.motor3_speed = motor_speed_commands_[2];
        msg.motor4_speed = motor_speed_commands_[3];
        
        // 设置机械臂关节角度（默认值）
        msg.joint1_angle = 0;
        msg.joint2_angle = 0;
        msg.joint3_angle = 0;
        msg.wrist_angle = 0;
        msg.gripper_angle = 0;
        
        // 设置复位标志
        msg.rest_flag = 0;
        
        // 发布速度命令到/robot_cmd话题
        motor_control_publisher_.publish(msg);
        
        // 打印调试信息
        ROS_DEBUG("电机速度指令: %d, %d, %d, %d", 
                 motor_speed_commands_[0], motor_speed_commands_[1], 
                 motor_speed_commands_[2], motor_speed_commands_[3]);
    }
    
    /**
     * @brief 发送零速度命令
     */
    void send_zero_speed()
    {
        serial_comm::RobotControl msg;
        msg.motor1_speed = 0;
        msg.motor2_speed = 0;
        msg.motor3_speed = 0;
        msg.motor4_speed = 0;
        msg.joint1_angle = 0;
        msg.joint2_angle = 0;
        msg.joint3_angle = 0;
        msg.wrist_angle = 0;
        msg.gripper_angle = 0;
        msg.rest_flag = 0;
        motor_control_publisher_.publish(msg);
    }
    
    // 常量定义
    static constexpr double RADIAN_COEF = 57.2957795131;  // 弧度转角度系数
    
    // 订阅者
    ros::Subscriber motor_states_subscriber_;
    ros::Subscriber imu_subscriber_;
    ros::Subscriber joy_subscriber_;
    
    // 发布者
    ros::Publisher motor_control_publisher_;
    
    // 参数
    double linear_scale_;      ///< 线性速度缩放因子
    double angular_scale_;     ///< 角速度缩放因子
    int max_speed_;            ///< 最大速度限制（RPM）
    
    // 电机状态
    double motor_positions_[4] = {0.0, 0.0, 0.0, 0.0};    ///< 电机位置
    double motor_velocities_[4] = {0.0, 0.0, 0.0, 0.0};   ///< 电机速度
    
    // 电机速度指令
    int16_t motor_speed_commands_[4] = {0, 0, 0, 0};      ///< 电机速度指令
    
    // IMU数据
    geometry_msgs::Quaternion imu_orientation_;      ///< 姿态四元数
    geometry_msgs::Vector3 imu_angular_velocity_;    ///< 角速度
    geometry_msgs::Vector3 imu_linear_acceleration_; ///< 线性加速度
    
    // 速度指令
    double vx_ = 0.0;    ///< 前进/后退速度
    double vy_ = 0.0;    ///< 左右平移速度
    double vw_ = 0.0;    ///< 旋转角速度
    
    // 控制模式
    ChassisCtrlMode_e ctrl_mode_;         ///< 当前控制模式
    ChassisCtrlMode_e last_ctrl_mode_;    ///< 上一次控制模式
    
    // 底盘参数
    MechanicalStructure_t structure_;     ///< 底盘机械结构参数
    Chassis_MotionControl motion_ctrl_;    ///< 底盘运动控制参数
};

/**
 * @brief 主函数
 */
int main(int argc, char *argv[])
{
    ros::init(argc, argv, "chassis_ctrl_node");
    ChassisCtrlNode node;
    node.spin();
    return 0;
}
