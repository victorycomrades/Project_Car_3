/**
 *****************************************************************
 * @file crc16.h
 * @brief CRC16计算头文件
 * 
 * 提供CRC16校验计算函数
 * 
 * @copyright Copyright (c) 2025 创非凡智能研究院
 ******************************************************************
 */
#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <cstddef>
/**
 * @brief 计算CRC16校验值
 * @param data 数据指针
 * @param length 数据长度
 * @return CRC16校验值
 */
uint16_t crc16_ibm(const uint8_t* data, size_t length);

#endif // CRC16_H