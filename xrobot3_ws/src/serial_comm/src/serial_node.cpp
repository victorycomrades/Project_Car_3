/**
 *****************************************************************
 * @file serial_node.cpp
 * @brief 串口通信节点
 * 
 * ROS1节点，用于与STM32微控制器通信，发布传感器数据和接收控制指令
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Range.h>
#include <geometry_msgs/Twist.h>
#include "serial_comm/RobotControl.h"
#include "serial_comm/serial_comm.h"
#include <tf/transform_datatypes.h>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

/**
 * @brief 串口通信节点类
 */
class SerialNode {
public:
    /**
     * @brief 构造函数
     */
    SerialNode() {
        ros::NodeHandle nh("~");
        
        // 获取参数
        std::string serial_port;
        int baud_rate;
        nh.param("serial_port", serial_port, std::string("/dev/XCOM3"));
        nh.param("baud_rate", baud_rate, 115200);
        
        // 初始化通信器
        communicator_ = std::unique_ptr<SerialComm>(new SerialComm(serial_port, baud_rate));
        
        // 打开串口
        if (!communicator_->open()) {
            ROS_ERROR("Failed to open serial port");
            return;
        }
        
        // 创建发布者
        imu_publisher_ = nh.advertise<sensor_msgs::Imu>("imu/data", 10);
        ultrasonic_publisher_ = nh.advertise<sensor_msgs::Range>("ultrasonic", 10);
        line_sensor_publisher_ = nh.advertise<std_msgs::String>("line_sensor", 10);
        
        // 创建订阅者 - 使用自定义消息类型
        cmd_robot_subscriber_ = nh.subscribe("/robot_cmd", 5, &SerialNode::robotCmdCallback, this);
        
        // 启动数据读取线程
        read_thread_ = std::thread(&SerialNode::readDataLoop, this);
        
        ROS_INFO("Serial node initialized with 50Hz publish rate and 50Hz control rate");
    }
    
    /**
     * @brief 析构函数
     */
    ~SerialNode() {
        running_ = false;
        if (read_thread_.joinable()) {
            read_thread_.join();
        }
        communicator_->close();
    }
    
    /**
     * @brief 运行节点
     */
    void spin() {
        ros::Rate rate(50); // 100Hz
        while (ros::ok()) {
            processDataCallback();
            ros::spinOnce();
            rate.sleep();
        }
    }

private:
    /**
     * @brief 数据读取循环 - 在单独线程中运行
     */
    void readDataLoop() {
        while (running_) {
            try {
                // 直接读取数据到缓存，不做其他处理
                communicator_->readSensorDataToBuffer();
                
                // 短暂休眠，避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::microseconds(10)); // 0.1ms
            } catch (const std::exception &e) {
                std::cerr << "Error in readDataLoop: " << e.what() << std::endl;
                // 即使发生异常，也要继续运行
                std::this_thread::sleep_for(std::chrono::microseconds(100)); // 1ms
            }
        }
    }
    
    /**
     * @brief 数据处理函数
     */
    void processDataCallback() {
        SensorData data;
        
        // 从通信器获取最新的传感器数据
        if (communicator_->getLatestSensorData(data)) {
            // 发布传感器数据
            publishSensorData(data);
            
            // 发送控制命令
            sendControlCommand();
        }
    }
    
    /**
     * @brief 发送控制命令函数
     */
    void sendControlCommand() {
        ZDTCommand command;
        bool has_command = false;
        
        { // 加锁作用域
            std::lock_guard<std::mutex> lock(command_mutex_);
            if (has_new_command_) {
                command = latest_command_;
                has_new_command_ = false;
                has_command = true;
            } else {
                // 即使没有新命令，也发送最新的命令
                command = latest_command_;
                has_command = true;
            }
        }
        
        if (has_command) {
            // 通过串口发送命令给STM32
            communicator_->sendZDTCommand(command);
        }
    }
    
    /**
     * @brief 发布传感器数据
     * @param data 传感器数据
     */
    void publishSensorData(const SensorData& data) {
        // 发布IMU数据
        sensor_msgs::Imu imu_msg;
        imu_msg.header.stamp = ros::Time::now();
        imu_msg.header.frame_id = "imu_link";
        
        float gyro_x = (float)data.gyro_x*2000/32768.0;
        float gyro_y = (float)data.gyro_y*2000/32768.0;
        float gyro_z = (float)data.gyro_z*2000/32768.0;

        float acc_x = (float)data.acc_x*16.0/32768.0;
        float acc_y = (float)data.acc_y*16.0/32768.0;
        float acc_z = (float)data.acc_z*16.0/32768.0;

        float angle_roll = (float)data.angle_roll*180/32768.0;
        float angle_pitch = (float)data.angle_pitch*180/32768.0;
        float angle_yaw = (float)data.angle_yaw*180/32768.0;

        // 设置方向（从欧拉角转换为四元数）
        // 将度数转换为弧度
        geometry_msgs::Quaternion quat = tf::createQuaternionMsgFromRollPitchYaw(
            angle_roll * M_PI / 180.0, angle_pitch * M_PI / 180.0, angle_yaw * M_PI / 180.0);
        
        imu_msg.orientation = quat;
        
        // 设置角速度
        imu_msg.angular_velocity.x = gyro_x;
        imu_msg.angular_velocity.y = gyro_y;
        imu_msg.angular_velocity.z = gyro_z;
        
        // 设置线性加速度
        imu_msg.linear_acceleration.x = acc_x;
        imu_msg.linear_acceleration.y = acc_y;
        imu_msg.linear_acceleration.z = acc_z;
        
        // 设置协方差矩阵（如果不确定精度，可以设为0）
        for (int i = 0; i < 9; i++) {
            imu_msg.orientation_covariance[i] = 0.0;
            imu_msg.angular_velocity_covariance[i] = 0.0;
            imu_msg.linear_acceleration_covariance[i] = 0.0;
        }
        
        imu_publisher_.publish(imu_msg);
        
        // 发布超声波数据
        for (int i = 0; i < 4; i++) {
            sensor_msgs::Range range_msg;
            range_msg.header.stamp = ros::Time::now();
            range_msg.header.frame_id = "ultrasonic_" + std::to_string(i+1);
            range_msg.radiation_type = sensor_msgs::Range::ULTRASOUND;
            range_msg.field_of_view = 0.5;
            range_msg.min_range = 0.02;
            range_msg.max_range = 4.0;
            
            switch (i) {
                case 0: range_msg.range = data.ultrasonic1 / 100.0; break;
                case 1: range_msg.range = data.ultrasonic2 / 100.0; break;
                case 2: range_msg.range = data.ultrasonic3 / 100.0; break;
                case 3: range_msg.range = data.ultrasonic4 / 100.0; break;
            }
            
            ultrasonic_publisher_.publish(range_msg);
        }
        
        // 发布线路传感器数据
        std_msgs::String line_msg;
        line_msg.data = std::to_string(data.line_sensor1) + "," +
                    std::to_string(data.line_sensor2) + "," +
                    std::to_string(data.line_sensor3) + "," +
                    std::to_string(data.line_sensor4);
        line_sensor_publisher_.publish(line_msg);
    }
    
    /**
     * @brief 机器人控制命令回调函数
     * @param msg 机器人控制消息
     */
    void robotCmdCallback(const serial_comm::RobotControl::ConstPtr& msg) {
        ZDTCommand command;
        
        // 设置电机速度
        command.motor1_speed = static_cast<int16_t>(msg->motor1_speed);
        command.motor2_speed = static_cast<int16_t>(msg->motor2_speed);
        command.motor3_speed = static_cast<int16_t>(msg->motor3_speed);
        command.motor4_speed = static_cast<int16_t>(msg->motor4_speed);
        
        std::cout << " motor1 " << command.motor1_speed << " " << std::endl;


        // 设置机械臂控制数据
        command.joint_angle[0]  = static_cast<int16_t>(msg->joint1_angle);
        command.joint_angle[1]  = static_cast<int16_t>(msg->joint2_angle);
        command.joint_angle[2]  = static_cast<int16_t>(msg->joint3_angle);
        command.wrist_angle = static_cast<int16_t>(msg->wrist_angle*10.0);
        command.gripper_angle = static_cast<int16_t>(msg->gripper_angle*10.0);
        
        // 设置复位标志和按钮
        command.rest_flag = msg->rest_flag;
        
        // 存储命令，由定时器统一发送
        std::lock_guard<std::mutex> lock(command_mutex_);
        latest_command_ = command;
        has_new_command_ = true;
    }
    
    std::unique_ptr<SerialComm> communicator_; ///< 串口通信器
    ros::Publisher imu_publisher_; ///< IMU数据发布者
    ros::Publisher ultrasonic_publisher_; ///< 超声波数据发布者
    ros::Publisher line_sensor_publisher_; ///< 线路传感器数据发布者
    ros::Subscriber cmd_robot_subscriber_; ///< 机器人控制命令订阅者

    std::thread read_thread_; ///< 数据读取线程
    std::atomic<bool> running_{true}; ///< 运行标志
    std::mutex command_mutex_; ///< 命令互斥锁
    ZDTCommand latest_command_; ///< 最新的控制命令
    bool has_new_command_ = false; ///< 是否有新命令标志
};

/**
 * @brief 主函数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 执行结果
 */
int main(int argc, char** argv) {
    ros::init(argc, argv, "serial_node");
    SerialNode node;
    node.spin();
    return 0;
}