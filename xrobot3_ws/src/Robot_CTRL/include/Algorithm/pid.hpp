/**
 *****************************************************************
 * @file pid.hpp
 * @brief PID控制器头文件
 * 
 * 提供PID控制器的定义和接口
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#ifndef PID_HPP
#define PID_HPP

#include <stdint.h>

/**
 * @brief PID模式枚举
 */
enum class PID_MODE {
    PID_POSITION = 0,  // 位置式PID
    PID_DELTA = 1      // 增量式PID
};

/**
 * @brief PID控制器结构体
 */
typedef struct {
    PID_MODE mode;      // PID模式
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    float max_output;   // 最大输出
    float min_output;   // 最小输出
    float setpoint;     // 设定值
    float feedback;     // 反馈值
    float output;       // 输出值
    float integral;     // 积分值
    float last_error;   // 上一次误差
    float last_last_error; // 上上次误差
} PidTypeDef;

/**
 * @brief 初始化PID控制器
 * @param pid PID控制器结构体指针
 * @param mode PID模式
 * @param params PID参数数组 [Kp, Ki, Kd]
 * @param max_output 最大输出
 * @param min_output 最小输出
 */
void PID_init(PidTypeDef* pid, PID_MODE mode, const float* params, float max_output, float min_output);

/**
 * @brief 计算PID输出
 * @param pid PID控制器结构体指针
 * @param feedback 反馈值
 * @param setpoint 设定值
 * @return PID输出值
 */
float PID_Calc(PidTypeDef* pid, float feedback, float setpoint);

#endif // PID_HPP