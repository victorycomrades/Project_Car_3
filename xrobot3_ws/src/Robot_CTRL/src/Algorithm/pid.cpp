/**
 *****************************************************************
 * @file pid.cpp
 * @brief PID控制器实现
 * 
 * 实现PID控制器的初始化和计算函数
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#include "Algorithm/pid.hpp"

/**
 * @brief 初始化PID控制器
 * @param pid PID控制器结构体指针
 * @param mode PID模式
 * @param params PID参数数组 [Kp, Ki, Kd]
 * @param max_output 最大输出
 * @param min_output 最小输出
 */
void PID_init(PidTypeDef* pid, PID_MODE mode, const float* params, float max_output, float min_output) {
    if (pid == nullptr || params == nullptr) {
        return;
    }
    
    pid->mode = mode;
    pid->Kp = params[0];
    pid->Ki = params[1];
    pid->Kd = params[2];
    pid->max_output = max_output;
    pid->min_output = min_output;
    pid->setpoint = 0.0f;
    pid->feedback = 0.0f;
    pid->output = 0.0f;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->last_last_error = 0.0f;
}

/**
 * @brief 计算PID输出
 * @param pid PID控制器结构体指针
 * @param feedback 反馈值
 * @param setpoint 设定值
 * @return PID输出值
 */
float PID_Calc(PidTypeDef* pid, float feedback, float setpoint) {
    if (pid == nullptr) {
        return 0.0f;
    }
    
    pid->feedback = feedback;
    pid->setpoint = setpoint;
    
    float error = setpoint - feedback;
    float output = 0.0f;
    
    if (pid->mode == PID_MODE::PID_POSITION) {
        // 位置式PID
        pid->integral += error;
        output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * (error - pid->last_error);
    } else {
        // 增量式PID
        output = pid->Kp * (error - pid->last_error) + 
                 pid->Ki * error + 
                 pid->Kd * (error - 2 * pid->last_error + pid->last_last_error);
        output += pid->output;
    }
    
    // 输出限幅
    if (output > pid->max_output) {
        output = pid->max_output;
    } else if (output < pid->min_output) {
        output = pid->min_output;
    }
    
    // 更新历史值
    pid->last_last_error = pid->last_error;
    pid->last_error = error;
    pid->output = output;
    
    return output;
}