/**
 *****************************************************************
 * @file crc16.cpp
 * @brief CRC16计算实现
 * 
 * 实现CRC16校验计算函数
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#include "serial_comm/crc16.h"
#include <cstddef>
/**
 * @brief 计算CRC16校验值
 * @param data 数据指针
 * @param length 数据长度
 * @return CRC16校验值
 */
uint16_t crc16_ibm(const uint8_t* data, size_t length) {
    uint16_t crc = 0x0000;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= (data[i] << 8);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x8005;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}