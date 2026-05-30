#ifndef __THREAD_SENSOR_H_
#define __THREAD_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* Includes ------------------------------------------------------------------*/
#include "mytype.h"
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* 传感器数据结构体 */
typedef union 
{
		#define UPLOAD_DATA_LEN 42
		uint8_t Buff[UPLOAD_DATA_LEN];
		struct
		{
			uint8_t header[2];      // 帧头 "SD"
			// 循线条数据（4个）
			uint8_t line_sensor1;   // 循线条1数据
			uint8_t line_sensor2;   // 循线条2数据
			uint8_t line_sensor3;   // 循线条3数据
			uint8_t line_sensor4;   // 循线条4数据
			// 超声波数据（4个）
			__packed uint16_t ultrasonic1;   // 超声波1距离
			__packed uint16_t ultrasonic2;   // 超声波2距离
			__packed uint16_t ultrasonic3;   // 超声波3距离
			__packed uint16_t ultrasonic4;   // 超声波4距离
			// 陀螺仪数据
			__packed int16_t gyro_x;        // 陀螺仪X轴数据
			__packed int16_t gyro_y;        // 陀螺仪Y轴数据
			__packed int16_t gyro_z;        // 陀螺仪Z轴数据

			
			__packed int16_t acc_x;         // 加速度X轴数据
			__packed int16_t acc_y;         // 加速度Y轴数据
			__packed int16_t acc_z;         // 加速度Z轴数据

			
			__packed int16_t angle_roll;       // 翻滚角
			__packed int16_t angle_pitch;      // 俯仰角
			__packed int16_t angle_yaw;        // 偏航角

			// 舵机编码器回传（4路）
			__packed int16_t servo1_pos;     // 舵机1编码器值（关节1）
			__packed int16_t servo2_pos;     // 舵机2编码器值（关节2）
			__packed int16_t servo3_pos;     // 舵机3编码器值（关节3）
			__packed int16_t servo4_pos;     // 舵机4编码器值（夹爪）

			__packed uint16_t crc;           // 校验和
		} DATE;
		
}SensorData_t;  //传感器数据结构体


/* ZDT电机指令结构体 */
typedef union 
{
		#define DOWNLOAD_DATA_LEN 23
		uint8_t Buff[DOWNLOAD_DATA_LEN];
		struct
		{
				uint8_t header[2];      // 帧头 "ZD"
				__packed int16_t motor1_speed;   // 电机1目标速度
				__packed int16_t motor2_speed;   // 电机2目标速度
				__packed int16_t motor3_speed;   // 电机3目标速度
				__packed int16_t motor4_speed;   // 电机4目标速度
			
				/**机械臂控制数据**/
				__packed int16_t joint_angle[3];     // 关节角度

				__packed int16_t wrist_angle;      // 手腕角度
				__packed int16_t gripper_angle;    // 夹抓角度
			
			
			  __packed uint8_t rest_flag;       //机械臂复位按钮
			
			

			
			
				__packed uint16_t crc;           // 校验和
			
		} DATE;
		
	
}RC_Command_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int Task_Sensor_create(void);
void sensor_uart_rx_callback(char* data, uint16_t len);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __THREAD_SENSOR_H_ */