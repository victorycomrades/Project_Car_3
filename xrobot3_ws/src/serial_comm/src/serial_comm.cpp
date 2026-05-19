/**
 *****************************************************************
 * @file serial_comm.cpp
 * @brief 串口通信类实现
 * 
 * 实现与STM32微控制器通信的类的方法，包括串口操作、数据读取和指令发送
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#include "serial_comm/serial_comm.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>

/**
 * @brief 构造函数
 * @param port 串口设备路径
 * @param baud_rate 波特率
 */
SerialComm::SerialComm(const std::string& port, int baud_rate)
    : port_(port), baud_rate_(baud_rate), is_open_(false) {
    // 初始化命令队列
}

/**
 * @brief 析构函数
 */
SerialComm::~SerialComm() {
    close();
}

/**
 * @brief 打开串口
 * @return 成功返回true，失败返回false
 */
bool SerialComm::open() {
    try {
        serial_port.Open(port_);
        serial_port.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
        serial_port.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
        serial_port.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
        serial_port.SetParity(LibSerial::Parity::PARITY_NONE);
        serial_port.SetStopBits(LibSerial::StopBits::STOP_BITS_1);
        is_open_ = true;
        std::cout << "Serial port " << port_ << " opened successfully." << std::endl;
        
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error opening serial port: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 关闭串口
 */
void SerialComm::close() {
    if (is_open_) {
        try {
            serial_port.Close();
            is_open_ = false;
            std::cout << "Serial port " << port_ << " closed." << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Error closing serial port: " << e.what() << std::endl;
        }
    }
}

/**
 * @brief 检查串口是否打开
 * @return 打开返回true，否则返回false
 */
bool SerialComm::isOpen() const {
    return is_open_;
}

/**
 * @brief 读取传感器数据到缓存
 * @return 成功返回true，失败返回false
 */
bool SerialComm::readSensorDataToBuffer() {
    std::lock_guard<std::mutex> lock(serial_mutex_);
    
    if (!is_open_) {
        std::cerr << "Serial port not open." << std::endl;
        return false;
    }
    
    try {
        // 等待帧头 "SD"
        uint8_t header[2];
        int bytes_read = 0;
        int error_count = 0;
        const int MAX_ERRORS = 100;
        const int MAX_WAIT_TIME = 500; // 最大等待时间（毫秒）
        auto start_time = std::chrono::steady_clock::now();
        
        while (bytes_read < 2) {
            // 检查是否超时
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > MAX_WAIT_TIME) {
                return false;
            }
            
            if (serial_port.IsDataAvailable()) {
                LibSerial::DataBuffer buffer;
                serial_port.Read(buffer, 1);
                if (!buffer.empty()) {
                    uint8_t byte = buffer[0];
                    if (bytes_read == 0) {
                        if (byte == 'S') {
                            header[bytes_read] = byte;
                            bytes_read++;
                        } else {
                            error_count++;
                            if (error_count > MAX_ERRORS) {
                                return false;
                            }
                        }
                    } else {
                        if (byte == 'D') {
                            header[bytes_read] = byte;
                            bytes_read++;
                        } else {
                            // 重新开始寻找帧头
                            bytes_read = 0;
                            error_count++;
                            if (error_count > MAX_ERRORS) {
                                return false;
                            }
                        }
                    }
                }
            } else {
                // 短暂休眠，避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::microseconds(100)); // 0.1ms
            }
        }
        
        // 读取剩余数据
        LibSerial::DataBuffer frame_data;
        serial_port.Read(frame_data, SENSOR_DATA_SIZE-2); // 34 - 2 = 32
        
        // 构建完整数据帧
        std::vector<uint8_t> full_frame(2 + frame_data.size());
        memcpy(full_frame.data(), header, 2);
        memcpy(full_frame.data() + 2, frame_data.data(), frame_data.size());
        
        // 解析数据并存储到缓存
        SensorData data;
        if (parseSensorData(full_frame, data)) {
            std::lock_guard<std::mutex> data_lock(data_mutex_);
            latest_sensor_data_ = data;
            has_new_data_ = true;
            return true;
        }
        
        return false;
    } catch (const std::exception &e) {
        std::cerr << "Error reading sensor data: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 从缓存获取最新的传感器数据
 * @param data 传感器数据结构体引用
 * @return 成功返回true，失败返回false
 */
bool SerialComm::getLatestSensorData(SensorData& data) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    if (!has_new_data_) {
        return false;
    }
    
    data = latest_sensor_data_;
    has_new_data_ = false;
    return true;
}

/**
 * @brief 解析传感器数据
 * @param frame 数据帧
 * @param data 传感器数据结构体引用
 * @return 成功返回true，失败返回false
 */
bool SerialComm::parseSensorData(const std::vector<uint8_t>& frame, SensorData& data) {
    // 验证CRC
    // 计算实际的CRC位置
    size_t crc_offset = SENSOR_DATA_SIZE-2; // 34 - 2 = 32
    // 读取CRC值（小端字节序）
    uint16_t received_crc = (frame[crc_offset + 1] << 8) | frame[crc_offset];
    
    // 计算CRC（与STM32代码完全一致）
    // 先复制数据到临时缓冲区，并将CRC字段设置为0
    std::vector<uint8_t> temp_frame = frame;
    temp_frame[crc_offset] = 0;
    temp_frame[crc_offset + 1] = 0;
    
    uint16_t calculated_crc = crc16_ibm(temp_frame.data(), crc_offset);
    
    if (received_crc != calculated_crc) {
        std::cerr << "CRC error: received=" << received_crc << ", calculated=" << calculated_crc << std::endl;
        // 暂时忽略CRC错误，继续解析数据
    }
    
    // 解析数据
    int offset = 2; // 跳过帧头
    data.line_sensor1 = frame[offset++];
    data.line_sensor2 = frame[offset++];
    data.line_sensor3 = frame[offset++];
    data.line_sensor4 = frame[offset++];
    
    // 处理小端字节序
    data.ultrasonic1 = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.ultrasonic2 = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.ultrasonic3 = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.ultrasonic4 = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    
    // 直接解析IMU数据，不需要跳过未知数据
    // 处理陀螺仪数据（int16_t，小端字节序）
    data.gyro_x = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.gyro_y = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.gyro_z = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    
    // 处理加速度数据（int16_t，小端字节序）
    data.acc_x = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.acc_y = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.acc_z = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    
    // 处理角度数据（int16_t，小端字节序）
    data.angle_roll = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.angle_pitch = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    data.angle_yaw = (frame[offset + 1] << 8) | frame[offset]; offset += 2;
    
    return true;
}

/**
 * @brief 发送ZDT控制指令
 * @param command ZDT指令结构体
 * @return 成功返回true，失败返回false
 */
bool SerialComm::sendZDTCommand(const ZDTCommand& command) {
    if (!is_open_) {
        std::cerr << "Serial port not open." << std::endl;
        return false;
    }
    
    try {
        // 将命令加入队列，在接收传感器数据时处理发送
        std::lock_guard<std::mutex> lock(command_queue_mutex_);
        command_queue_.push(command);
        
        // 发送命令
        sendCommandInternal(command);

        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error queuing ZDT command: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 处理命令队列，发送控制指令
 */
void SerialComm::processCommandQueue() {
    // 检查是否有命令需要发送
    ZDTCommand command;
    bool has_command = false;
    
    {
        std::lock_guard<std::mutex> lock(command_queue_mutex_);
        if (!command_queue_.empty()) {
            command = command_queue_.front();
            command_queue_.pop();
            has_command = true;
        }
    }
    
    if (has_command) {
        // 发送命令
        sendCommandInternal(command);
    }
}

/**
 * @brief 内部发送命令函数
 * @param command ZDT指令结构体
 * @return 成功返回true，失败返回false
 */
bool SerialComm::sendCommandInternal(const ZDTCommand& command) {
    try {
        // 构建指令帧
        std::vector<uint8_t> frame(ZDT_COMMAND_SIZE);
        
        // 帧头
        frame[0] = 'Z';
        frame[1] = 'D';
        
        // 电机速度 - 小端字节序
        int offset = 2;
        int16_t temp_int16;
        
        // Motor 1
        temp_int16 = command.motor1_speed;
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Motor 2
        temp_int16 = command.motor2_speed;
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Motor 3
        temp_int16 = command.motor3_speed;
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Motor 4
        temp_int16 = command.motor4_speed;
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // 机械臂关节角度 - 小端字节序
        
        // Joint 1
        temp_int16 = command.joint_angle[0];
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Joint 2
        temp_int16 = command.joint_angle[1];
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Joint 3
        temp_int16 = command.joint_angle[2];
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Wrist
        temp_int16 = command.wrist_angle;
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Gripper
        temp_int16 = command.gripper_angle;
        frame[offset++] = temp_int16 & 0xFF;
        frame[offset++] = (temp_int16 >> 8) & 0xFF;
        
        // Rest flag
        frame[offset++] = command.rest_flag;
        
        // 计算CRC - CRC只包含帧头和数据部分，不包含CRC本身
        uint16_t crc = crc16_ibm(frame.data(), ZDT_COMMAND_SIZE - 2);
        
        // 将CRC以小端字节序写入帧末尾
        frame[ZDT_COMMAND_SIZE - 2] = crc & 0xFF;
        frame[ZDT_COMMAND_SIZE - 1] = (crc >> 8) & 0xFF;
        

        // 发送数据
        LibSerial::DataBuffer buffer(frame.begin(), frame.end());
        serial_port.Write(buffer);
        
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error sending ZDT command: " << e.what() << std::endl;
        return false;
    }
}