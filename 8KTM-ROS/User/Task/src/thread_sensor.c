/* Includes ------------------------------------------------------------------*/
#include "thread_sensor.h"
/* �����������ļ� */
#include "data.h"

/* ����ģ��ͷ�ļ� */
#include "bsp_hal_uartdma.h"
#include "chassis_LineTracker.h"

#include "fifo.h"
#include "thread_comm.h"

/* HAL��ͷ�ļ� */
#include "stm32f4xx_hal_uart.h"
/* ������ģ��ͷ�ļ� */
#include "SLAVE_UltrasonicRanging/SLAVE_UltrasonicRanging.h"

/* �ַ�������ͷ�ļ� */
#include <string.h>
#include <stdio.h>

/*���*/
#include "Servo.h"
#include "SMS_STS.h"
#include "bsp_hal_st7789.h"
#include "thread_rccu.h"
#include "SCARA.h"
#include <stdlib.h>


/* �ⲿ�������� */
extern UartMixed_TypeDef muart2;
extern RCCUStruct_TypeDef rccu_struct;

/* Private macros ------------------------------------------------------------*/
#define SENSOR_DATA_INTERVAL 40 // ���������ݷ��ͼ��(ms)
#define UART_MIXED &muart2 // ʹ��muart3��������
#define RX_BUFFER_SIZE 128 // ���ջ�������С
/* Private types -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* �����߳̿��ƿ�ָ�� */
rt_thread_t thread_SensorData = RT_NULL;
rt_thread_t thread_ZDTCommand = RT_NULL;
rt_thread_t thread_ARMCommand = RT_NULL;
/* ���ջ����� */
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;
/* ������Ʊ�־ */
uint8_t motor_control_flag = 0;


SensorData_t sensor_data;

/* ȫ��ZDT����ṹ�� */
RC_Command_t g_rc_command;


#define PC_FIFO_BUF_LENGTH 1024

uint16_t rc_cnt = 0;

uint16_t arm_cnt = 0;

fifo_s_t pc_fifo;
uint8_t  pc_fifo_buf[PC_FIFO_BUF_LENGTH];
/* Private functions ---------------------------------------------------------*/
static int32_t CFF_GetMotorPosition(uint8_t motor_id);
static void SensorData_Send(void);
static uint16_t crc16_ibm(const uint8_t *data, int len);
static void process_zdt_command(uint8_t *data);
static void sensor_uart_tx_callback(void);
static void CFFCommand_task(void *pvParameters);

static void SensorData_Send(void)
{

    
    /* ���֡ͷ */
    sensor_data.DATE.header[0] = 'S';
    sensor_data.DATE.header[1] = 'D';
    
    /* ���ѭ�������ݣ�4���� */
    sensor_data.DATE.line_sensor1 = Tracking_Device[0].Tracking_UploadData.DATE.SignalData;
    sensor_data.DATE.line_sensor2 = Tracking_Device[1].Tracking_UploadData.DATE.SignalData;
    sensor_data.DATE.line_sensor3 = Tracking_Device[2].Tracking_UploadData.DATE.SignalData;
    sensor_data.DATE.line_sensor4 = Tracking_Device[3].Tracking_UploadData.DATE.SignalData;
    
    /* ��䳬�������ݣ�4���� */
#ifdef __SLAVE_UltrasonicRanging_H__
    sensor_data.DATE.ultrasonic1 = UltrasonicRanging_S.UltrasonicRanging_UploadData1.DATE.Distance1;
    sensor_data.DATE.ultrasonic2 = UltrasonicRanging_S.UltrasonicRanging_UploadData1.DATE.Distance2;
    sensor_data.DATE.ultrasonic3 = UltrasonicRanging_S.UltrasonicRanging_UploadData1.DATE.Distance3;
    sensor_data.DATE.ultrasonic4 = UltrasonicRanging_S.UltrasonicRanging_UploadData1.DATE.Distance4;
#else
    sensor_data.DATE.ultrasonic1 = 0;
    sensor_data.DATE.ultrasonic2 = 0;
    sensor_data.DATE.ultrasonic3 = 0;
    sensor_data.DATE.ultrasonic4 = 0;
#endif
    
    /* ������������� */
//    sensor_data.DATE.gyro_x = GyroData_Struct.wx;
//    sensor_data.DATE.gyro_y = GyroData_Struct.wy;
//    sensor_data.DATE.gyro_z = GyroData_Struct.wz;
//    sensor_data.DATE.acc_x = GyroData_Struct.ax;
//    sensor_data.DATE.acc_y = GyroData_Struct.ay;
//    sensor_data.DATE.acc_z = GyroData_Struct.az;
//    sensor_data.DATE.angle_roll = GyroData_Struct.Roll;
//    sensor_data.DATE.angle_pitch = GyroData_Struct.Pitch;
//    sensor_data.DATE.angle_yaw = GyroData_Struct.Yaw;
		
    sensor_data.DATE.gyro_x = GyroData_Struct.stcGyro[0];
    sensor_data.DATE.gyro_y = GyroData_Struct.stcGyro[1];
    sensor_data.DATE.gyro_z = GyroData_Struct.stcGyro[2];
    sensor_data.DATE.acc_x = GyroData_Struct.stcAcc[0];
    sensor_data.DATE.acc_y = GyroData_Struct.stcAcc[1];
    sensor_data.DATE.acc_z = GyroData_Struct.stcAcc[2];
    sensor_data.DATE.angle_roll = GyroData_Struct.stcAngle[0];
    sensor_data.DATE.angle_pitch = GyroData_Struct.stcAngle[1];
    sensor_data.DATE.angle_yaw = GyroData_Struct.stcAngle[2];
		
    
    /* ���ZDT������ݣ�4���� */
//    sensor_data.DATE.motor1_pos = ZDT_GetMotorPosition(1);
//    sensor_data.DATE.motor2_pos = ZDT_GetMotorPosition(2);
//    sensor_data.DATE.motor3_pos = ZDT_GetMotorPosition(3);
//    sensor_data.DATE.motor4_pos = ZDT_GetMotorPosition(4);
    
    /* ����У��� */
    uint8_t *data_ptr = (uint8_t *)&sensor_data;
    sensor_data.DATE.crc = 0;
    sensor_data.DATE.crc = crc16_ibm(data_ptr, sizeof(SensorData_t) - 2); // ��ȥCRC�ֶε�2���ֽ�
    
    /* �������� */
    Bsp_UARTMixed_TxTrigger(UART_MIXED, (char *)&sensor_data, sizeof(SensorData_t));
}

static uint16_t crc16_ibm(const uint8_t *data, int len) {
    uint16_t crc = 0x0000; // ��ʼֵ
    for (; len > 0; len--) {
        crc ^= (*data++) << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x8005; // ����ʽ
            else
                crc <<= 1;
        }
    }
    return crc; // �������ת�����
}

static void process_zdt_command(uint8_t *data)
{
    RC_Command_t *cmd = (RC_Command_t *)data;
    
    /* ��֤У��� */
    uint16_t crc = crc16_ibm(data, sizeof(RC_Command_t) - 2); // ��ȥCRC�ֶε�2���ֽ�
    if (crc != cmd->DATE.crc) {
        return; // У��ʹ��󣬶�������
    }
		
		 /* �����յ������ݱ��浽ȫ�ֽṹ����� */
    memcpy(&g_rc_command, data, sizeof(RC_Command_t));
    

		
}

static int32_t CFF_GetMotorPosition(uint8_t motor_id)
{
    // ������Ҫʵ�ֶ�ȡ���λ�õĹ���
    // ����û��ֱ�ӵĺ��������ǿ���ʹ��ZDT_ReadSysParams����
    // ʵ��Ӧ������Ҫ����Ӳ���ӿ�ʵ��ͨ��
    return 0; // ��ʱ����0����Ҫ����ʵ�����ʵ��
}


char _lcd_text[64];
static uint8_t _lcd_idx = 0;
uint8_t _lcd_ready = 0;

void sensor_uart_rx_callback(char* data, uint16_t len)
{
    uint16_t i;
    static char _linebuf[128];
    static uint8_t _li = 0;

    fifo_s_puts(&pc_fifo, (char*)data, len);

    for (i = 0; i < len; i++) {
        char c = data[i];
        if (c == '\n' || c == '\r') {
            if (_li > 0) {
                _linebuf[_li] = '\0';

                /* ----- display command ----- */
                if (_linebuf[0] == '{' && strstr(_linebuf, "\"cmd\":\"display\"") && strstr(_linebuf, "\"text\":\"")) {
                    char *p = strstr(_linebuf, "\"text\":\"");
                    p += 8;
                    uint8_t j = 0;
                    while (*p && *p != '\"' && j < sizeof(_lcd_text) - 1)
                        _lcd_text[j++] = *p++;
                    _lcd_text[j] = '\0';
                    _lcd_ready = 1;
                }

                /* ----- nav command ----- */
                if (_linebuf[0] == '{' && strstr(_linebuf, "\"cmd\":\"nav\"")) {
                    char *px = strstr(_linebuf, "\"linear_x\":");
                    char *py = strstr(_linebuf, "\"linear_y\":");
                    char *pz = strstr(_linebuf, "\"angular_z\":");
                    if (px && py && pz) {
                        float vx = 0, vy = 0, vw = 0;
                        sscanf(px, "\"linear_x\":%f", &vx);
                        sscanf(py, "\"linear_y\":%f", &vy);
                        sscanf(pz, "\"angular_z\":%f", &vw);
                        rccu_struct.chassis_struct.vx = vx;
                        rccu_struct.chassis_struct.vy = vy;
                        rccu_struct.chassis_struct.vw = vw;
                        if (rccu_struct.mode_run == CHASSIS_RELAX || rccu_struct.mode_run == CHASSIS_STOP) {
                            rccu_struct.mode_order = CHASSIS_NORMAL;
                        }
                        Bsp_UARTMixed_TxTrigger(UART_MIXED, "NAVOK\n", 6);
                    }
                }

                /* ----- arm command (absolute position) ----- */
                // TODO: add SCARA.c to project build, then uncomment below
                // if (_linebuf[0] == '{' && strstr(_linebuf, "\"cmd\":\"arm\"")) {
                //     char *px = strstr(_linebuf, "\"x\":");
                //     char *py = strstr(_linebuf, "\"y\":");
                //     if (px && py) {
                //         float cartesian[3];
                //         uint16_t pwm1, pwm2, pwm3;
                //         cartesian[0] = atof(px + 4);
                //         cartesian[1] = atof(py + 4);
                //         cartesian[2] = 0;
                //         calculate_delta(cartesian);
                //         Servo_AngleToPWM(SCARA_S.Angle, &pwm1, &pwm2, &pwm3);
                //         WritePosEx(1, pwm1, 1000, 160);
                //         WritePosEx(2, pwm2, 1000, 160);
                //         WritePosEx(3, pwm3, 1000, 160);
                //     }
                // }

                _li = 0;
            }
        } else if (_li < sizeof(_linebuf) - 1) {
            _linebuf[_li++] = c;
        }
    }
}

static void sensor_uart_tx_callback(void)
{
    // ������ɻص�����������һЩ�����߼�
}


/**
  * @brief          ���ֽڽ��
  * @param[in]      void
  * @retval         none
  */
void PC_unpack_fifo_data(void)
{
	
  uint8_t byte = 0;

  while ( fifo_s_used(&pc_fifo) )
  {
	  byte = fifo_s_get(&pc_fifo);
		
			  // ���ӱ߽���
        if (rx_index >= RX_BUFFER_SIZE) {
            rx_index = 0; // ���û�����
        }
		
        rx_buffer[rx_index++] = byte;
        
				//Ѱ��֡ͷ
				if(rx_index >= 2)
				{
				  if(rx_buffer[rx_index-2] == 'Z' && rx_buffer[rx_index-1] == 'D')
					{
					   memmove(rx_buffer,&rx_buffer[rx_index-2], 2);
						rx_index = 2;
						
					}
				
				}
				
        /* ����Ƿ��յ�������ZDTָ��֡ */
        if (rx_index >= sizeof(RC_Command_t)) {
            /* ���֡ͷ */
            if (rx_buffer[0] == 'Z' && rx_buffer[1] == 'D') {
                process_zdt_command(rx_buffer);
            }
            rx_index = 0; // ���ý�������
        }
		
	}	
	
}

void ZDT_chassis_ctrl(RC_Command_t *cmd)
{

		switch (rc_cnt)
		{
		case 0:
			  SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt1_S,cmd->DATE.motor1_speed);
				rc_cnt ++;
				break;
		case 1:
			  SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt2_S,cmd->DATE.motor2_speed);
				rc_cnt ++;
				break;
		case 2:
			  SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt3_S,cmd->DATE.motor3_speed);
				rc_cnt ++;
				break;
		case 3:
			  SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt4_S,cmd->DATE.motor4_speed);
				rc_cnt ++;
				break;

		default:
				rc_cnt = 0;
				break;
		}

}


void arm_ctrl(RC_Command_t *cmd)
{
	
	 float joint_angle[3],wrist_angle,gripper_angle;
	
   static uint8_t last_rest_flag=0;
		switch (arm_cnt)
		{
		case 0:
			  wrist_angle = (float)cmd->DATE.wrist_angle / 10.0f;
		
				//RobotArm_WristSetAngle(wrist_angle);
				arm_cnt ++;
				break;
		case 1:
			
		    gripper_angle = (float)cmd->DATE.gripper_angle / 10.0f;
		    WritePosEx( 4, gripper_angle, 1000, 160);
				//RobotArm_GripperSetAngle(cmd->DATE.gripper_angle);
				arm_cnt ++;
				break;
		case 2:
			  for(uint8_t i=0;i<3;i++)
				{
				   joint_angle[i] = cmd->DATE.joint_angle[i];
				}
				WritePosEx( 1, joint_angle[0], 1000, 160);
				WritePosEx( 2, joint_angle[1], 1000, 160);
				WritePosEx( 3, joint_angle[2], 1000, 160);
				//RobotArm_SetAngle(joint_angle[0], joint_angle[1], joint_angle[2]);
				arm_cnt ++;
				break;
		case 3:
			
		    if((cmd->DATE.rest_flag == 1)&&(last_rest_flag != cmd->DATE.rest_flag))
				{
					Rest_action_S2();
				  //RobotArm_Rst();
					My_mDelay(200);
				}
				last_rest_flag = cmd->DATE.rest_flag;
				arm_cnt ++;
				break;
		
		default:
				arm_cnt = 0;
				break;
		}
	
}


static void CFFCommand_task(void *pvParameters)
{
	
	  fifo_s_init(&pc_fifo, pc_fifo_buf, PC_FIFO_BUF_LENGTH);
    while(1)
    {
			  PC_unpack_fifo_data();
			
				ZDT_chassis_ctrl(&g_rc_command);
        rt_thread_mdelay(10);
    }
}

static void Robot_arm_task(void *pvParameters)
{
    //Rest_action_S2();
	  My_mDelay(200);
	  g_rc_command.DATE.joint_angle[0] = 1040;
	  g_rc_command.DATE.joint_angle[1] = 2140;
	  g_rc_command.DATE.joint_angle[2] = 2280;
    while(1)
    {
        //arm_ctrl(&g_rc_command);
        rt_thread_mdelay(50);
    }
}

/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void SensorData_task(void *pvParameters)
{
    while(1)
    {
        {
            static int _cnt = 0;
            if (++_cnt >= 10) {
                _cnt = 0;
                static char _wbuf[64];
                int len = snprintf(_wbuf, sizeof(_wbuf),
                    "{\"wheel\":{\"x\":%d,\"y\":%d,\"z\":%d,\"r\":%d}}\n",
                    (int)pos_x, (int)pos_y, (int)zangle, Wheel_isReady);
                if (len > 0 && len < (int)sizeof(_wbuf)) {
                    Bsp_UARTMixed_TxTrigger(UART_MIXED, _wbuf, len);
                    rt_thread_mdelay(5);
                }
            }
        }
        SensorData_Send();
        rt_thread_mdelay(SENSOR_DATA_INTERVAL);
    }
}

int Task_Sensor_create(void)
{
    /* ��ʼ��muart3 */
    //Bsp_UartMixed_Init(UART_MIXED, sensor_uart_rx_callback, sensor_uart_tx_callback);
    rt_kprintf("muart3 initialized!\n");
    
    /* �������������ݷ����߳� */
    thread_SensorData = rt_thread_create( "SensorData",             /* �߳����� */
                                        SensorData_task,          /* �߳���ں��� */
                                        RT_NULL,                 /* �߳���ں������� */
                                        1024,          		  /* �߳�ջ��С */
                                        15,                      /* �̵߳����ȼ� */
                                        20);                     /* �߳�ʱ��Ƭ */
    if(thread_SensorData != RT_NULL)
    {
        rt_thread_startup(thread_SensorData);
        rt_kprintf("thread_SensorData startup!\n");
    }
    
    /* ����ZDT���ָ������߳� */
    thread_ZDTCommand = rt_thread_create( "CFFCommand",             /* �߳����� */
                                         CFFCommand_task,          /* �߳���ں��� */
                                         RT_NULL,                 /* �߳���ں������� */
                                         512,          		  /* �߳�ջ��С */
                                         14,                      /* �̵߳����ȼ����ȴ����������̸߳ߣ� */
                                         20);                     /* �߳�ʱ��Ƭ */
    if(thread_ZDTCommand != RT_NULL)
    {
        rt_thread_startup(thread_ZDTCommand);
        rt_kprintf("thread_ZDTCommand startup!\n");
    }
		
    /* ������е�۵��ָ������߳� */
    thread_ARMCommand = rt_thread_create( "ARMCommand",             /* �߳����� */
                                         Robot_arm_task,          /* �߳���ں��� */
                                         RT_NULL,                 /* �߳���ں������� */
                                         512,          		  /* �߳�ջ��С */
                                         13,                      /* �̵߳����ȼ� */
                                         20);                     /* �߳�ʱ��Ƭ */
    if(thread_ARMCommand != RT_NULL)
    {
        rt_thread_startup(thread_ARMCommand);
        rt_kprintf("thread_ARMCommand startup!\n");
    }
    
    return 0;
}

INIT_APP_EXPORT(Task_Sensor_create);
