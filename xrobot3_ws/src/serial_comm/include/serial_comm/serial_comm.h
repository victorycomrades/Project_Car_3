/**
 *****************************************************************
 * @file serial_comm.h
 * @brief 串口通信类声明
 * 
 * 声明与STM32微控制器通信的类，包括串口操作、数据读取和指令发送
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "serial_comm/crc16.h"
#include <libserial/SerialPort.h>
#include <string>
#include <memory>
#include <mutex>
#include <queue>

// 与单片机端匹配的结构体定义
struct ZDTCommand {
    int16_t motor1_speed;
    int16_t motor2_speed;
    int16_t motor3_speed;
    int16_t motor4_speed;
    int16_t joint_angle[3];

    int16_t wrist_angle;
    int16_t gripper_angle;
    uint8_t rest_flag;
};

struct SensorData {
    uint8_t line_sensor1, line_sensor2, line_sensor3, line_sensor4;
    uint16_t ultrasonic1, ultrasonic2, ultrasonic3, ultrasonic4;
    int16_t gyro_x, gyro_y, gyro_z;
    int16_t acc_x, acc_y, acc_z;
    int16_t angle_roll, angle_pitch, angle_yaw;
};

// 传感器数据帧大小定义
#define SENSOR_DATA_SIZE 34  // 根据实际命令结构计算

// ZDT命令帧大小定义
#define ZDT_COMMAND_SIZE 23  // 根据实际命令结构计算

/**
 * @brief 串口通信类
 */
class SerialComm {
public:
    /**
     * @brief 构造函数
     * @param port 串口设备路径
     * @param baud_rate 波特率
     */
    SerialComm(const std::string& port, int baud_rate);
    
    /**
     * @brief 析构函数
     */
    ~SerialComm();
    
    /**
     * @brief 打开串口
     * @return 成功返回true，失败返回false
     */
    bool open();
    
    /**
     * @brief 关闭串口
     */
    void close();
    
    /**
     * @brief 检查串口是否打开
     * @return 打开返回true，否则返回false
     */
    bool isOpen() const;
    
    /**
     * @brief 读取传感器数据到缓存
     * @return 成功返回true，失败返回false
     */
    bool readSensorDataToBuffer();
    
    /**
     * @brief 从缓存获取最新的传感器数据
     * @param data 传感器数据结构体引用
     * @return 成功返回true，失败返回false
     */
    bool getLatestSensorData(SensorData& data);
    
    /**
     * @brief 发送ZDT控制指令
     * @param command ZDT指令结构体
     * @return 成功返回true，失败返回false
     */
    bool sendZDTCommand(const ZDTCommand& command);
    
    /**
     * @brief 处理命令队列，发送控制指令
     */
    void processCommandQueue();

private:
    /**
     * @brief 内部发送命令函数
     * @param command ZDT指令结构体
     * @return 成功返回true，失败返回false
     */
    bool sendCommandInternal(const ZDTCommand& command);
    
    /**
     * @brief 解析传感器数据
     * @param frame 数据帧
     * @param data 传感器数据结构体引用
     * @return 成功返回true，失败返回false
     */
    bool parseSensorData(const std::vector<uint8_t>& frame, SensorData& data);
    
    std::string port_; ///< 串口设备路径
    int baud_rate_; ///< 波特率
    LibSerial::SerialPort serial_port; ///< 串口对象
    bool is_open_; ///< 串口打开标志
    std::mutex serial_mutex_; ///< 串口操作互斥锁
    std::mutex command_queue_mutex_; ///< 命令队列互斥锁
    std::mutex data_mutex_; ///< 数据缓存互斥锁
    std::queue<ZDTCommand> command_queue_; ///< 命令队列
    SensorData latest_sensor_data_; ///< 最新的传感器数据
    bool has_new_data_; ///< 是否有新数据标志
};

#endif // SERIAL_COMM_H