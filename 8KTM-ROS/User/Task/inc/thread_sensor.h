#ifndef __THREAD_SENSOR_H_
#define __THREAD_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* Includes ------------------------------------------------------------------*/
#include "mytype.h"
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* ���������ݽṹ�� */
typedef union 
{
		#define UPLOAD_DATA_LEN 34
		uint8_t Buff[UPLOAD_DATA_LEN];
		struct
		{
			uint8_t header[2];      // ֡ͷ "SD"
			// ѭ�������ݣ�4����
			uint8_t line_sensor1;   // ѭ����1����
			uint8_t line_sensor2;   // ѭ����2����
			uint8_t line_sensor3;   // ѭ����3����
			uint8_t line_sensor4;   // ѭ����4����
			// ���������ݣ�4����
			__packed uint16_t ultrasonic1;   // ������1����
			__packed uint16_t ultrasonic2;   // ������2����
			__packed uint16_t ultrasonic3;   // ������3����
			__packed uint16_t ultrasonic4;   // ������4����
			// ����������
			__packed int16_t gyro_x;        // ������X������
			__packed int16_t gyro_y;        // ������Y������
			__packed int16_t gyro_z;        // ������Z������

			
			__packed int16_t acc_x;         // ���ٶ�X������
			__packed int16_t acc_y;         // ���ٶ�Y������
			__packed int16_t acc_z;         // ���ٶ�Z������

			
			__packed int16_t angle_roll;       // ������
			__packed int16_t angle_pitch;      // ������
			__packed int16_t angle_yaw;        // ƫ����

			__packed uint16_t crc;           // У���
		} DATE;
		
}SensorData_t;  //���������ݽṹ��


/* ZDT���ָ��ṹ�� */
typedef union 
{
		#define DOWNLOAD_DATA_LEN 23
		uint8_t Buff[DOWNLOAD_DATA_LEN];
		struct
		{
				uint8_t header[2];      // ֡ͷ "ZD"
				__packed int16_t motor1_speed;   // ���1Ŀ���ٶ�
				__packed int16_t motor2_speed;   // ���2Ŀ���ٶ�
				__packed int16_t motor3_speed;   // ���3Ŀ���ٶ�
				__packed int16_t motor4_speed;   // ���4Ŀ���ٶ�
			
				/**��е�ۿ�������**/
				__packed int16_t joint_angle[3];     // �ؽڽǶ�

				__packed int16_t wrist_angle;      // ����Ƕ�
				__packed int16_t gripper_angle;    // ��ץ�Ƕ�
			
			
			  __packed uint8_t rest_flag;       //��е�۸�λ��ť
			
			

			
			
				__packed uint16_t crc;           // У���
			
		} DATE;
		
	
}RC_Command_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern char _lcd_text[64];
extern uint8_t _lcd_ready;
/* Exported functions --------------------------------------------------------*/
int Task_Sensor_create(void);
void sensor_uart_rx_callback(char* data, uint16_t len);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __THREAD_SENSOR_H_ */