/* Includes ------------------------------------------------------------------*/
#include "thread_sensor.h"
/* 开发板数据文件 */
#include "data.h"

/* 功能模块头文件 */
#include "bsp_hal_uartdma.h"
#include "chassis_LineTracker.h"

#include "fifo.h"

/* HAL库头文件 */
#include "stm32f4xx_hal_uart.h"
/* 超声波模块头文件 */
#include "SLAVE_UltrasonicRanging/SLAVE_UltrasonicRanging.h"

/* 字符串操作头文件 */
#include <string.h>

/*舵机*/
#include "Servo.h"
#include "SMS_STS.h"


/* 外部变量声明 */
extern UartMixed_TypeDef muart2;

/* Private macros ------------------------------------------------------------*/
#define SENSOR_DATA_INTERVAL 40 // 传感器数据发送间隔(ms)
#define UART_MIXED &muart2 // 使用muart3发送数据
#define RX_BUFFER_SIZE 128 // 接收缓冲区大小
/* Private types -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* 定义线程控制块指针 */
rt_thread_t thread_SensorData = RT_NULL;
rt_thread_t thread_ZDTCommand = RT_NULL;
rt_thread_t thread_ARMCommand = RT_NULL;
/* 接收缓冲区 */
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;
/* 电机控制标志 */
uint8_t motor_control_flag = 0;


SensorData_t sensor_data;

/* 全局ZDT命令结构体 */
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

    
    /* 填充帧头 */
    sensor_data.DATE.header[0] = 'S';
    sensor_data.DATE.header[1] = 'D';
    
    /* 填充循线条数据（4个） */
    sensor_data.DATE.line_sensor1 = Tracking_Device[0].Tracking_UploadData.DATE.SignalData;
    sensor_data.DATE.line_sensor2 = Tracking_Device[1].Tracking_UploadData.DATE.SignalData;
    sensor_data.DATE.line_sensor3 = Tracking_Device[2].Tracking_UploadData.DATE.SignalData;
    sensor_data.DATE.line_sensor4 = Tracking_Device[3].Tracking_UploadData.DATE.SignalData;
    
    /* 填充超声波数据（4个） */
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
    
    /* 填充陀螺仪数据 */
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
		
    
    /* 填充ZDT电机数据（4个） */
//    sensor_data.DATE.motor1_pos = ZDT_GetMotorPosition(1);
//    sensor_data.DATE.motor2_pos = ZDT_GetMotorPosition(2);
//    sensor_data.DATE.motor3_pos = ZDT_GetMotorPosition(3);
//    sensor_data.DATE.motor4_pos = ZDT_GetMotorPosition(4);
    
    /* 计算校验和 */
    uint8_t *data_ptr = (uint8_t *)&sensor_data;
    sensor_data.DATE.crc = 0;
    sensor_data.DATE.crc = crc16_ibm(data_ptr, sizeof(SensorData_t) - 2); // 减去CRC字段的2个字节
    
    /* 发送数据 */
    Bsp_UARTMixed_TxTrigger(UART_MIXED, (char *)&sensor_data, sizeof(SensorData_t));
}

static uint16_t crc16_ibm(const uint8_t *data, int len) {
    uint16_t crc = 0x0000; // 初始值
    for (; len > 0; len--) {
        crc ^= (*data++) << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x8005; // 多项式
            else
                crc <<= 1;
        }
    }
    return crc; // 无输出反转和异或
}

static void process_zdt_command(uint8_t *data)
{
    RC_Command_t *cmd = (RC_Command_t *)data;
    
    /* 验证校验和 */
    uint16_t crc = crc16_ibm(data, sizeof(RC_Command_t) - 2); // 减去CRC字段的2个字节
    if (crc != cmd->DATE.crc) {
        return; // 校验和错误，丢弃数据
    }
		
		 /* 将接收到的数据保存到全局结构体变量 */
    memcpy(&g_rc_command, data, sizeof(RC_Command_t));
    

		
}

static int32_t CFF_GetMotorPosition(uint8_t motor_id)
{
    // 这里需要实现读取电机位置的功能
    // 由于没有直接的函数，我们可以使用ZDT_ReadSysParams函数
    // 实际应用中需要根据硬件接口实现通信
    return 0; // 暂时返回0，需要根据实际情况实现
}


void sensor_uart_rx_callback(char* data, uint16_t len)
{
	
	fifo_s_puts(&pc_fifo, (char*)data, len);
	
}

static void sensor_uart_tx_callback(void)
{
    // 发送完成回调，可以添加一些处理逻辑
}


/**
  * @brief          单字节解包
  * @param[in]      void
  * @retval         none
  */
void PC_unpack_fifo_data(void)
{
	
  uint8_t byte = 0;

  while ( fifo_s_used(&pc_fifo) )
  {
	  byte = fifo_s_get(&pc_fifo);
		
			  // 添加边界检查
        if (rx_index >= RX_BUFFER_SIZE) {
            rx_index = 0; // 重置缓冲区
        }
		
        rx_buffer[rx_index++] = byte;
        
				//寻找帧头
				if(rx_index >= 2)
				{
				  if(rx_buffer[rx_index-2] == 'Z' && rx_buffer[rx_index-1] == 'D')
					{
					   memmove(rx_buffer,&rx_buffer[rx_index-2], 2);
						rx_index = 2;
						
					}
				
				}
				
        /* 检查是否收到完整的ZDT指令帧 */
        if (rx_index >= sizeof(RC_Command_t)) {
            /* 检查帧头 */
            if (rx_buffer[0] == 'Z' && rx_buffer[1] == 'D') {
                process_zdt_command(rx_buffer);
            }
            rx_index = 0; // 重置接收索引
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
{//传感器数据发送任务
    while(1)
    {
        /* 发送传感器数据 */
        SensorData_Send();
        
        /* 延时 */
        rt_thread_mdelay(SENSOR_DATA_INTERVAL);
    }
}
int Task_Sensor_create(void)
{
    /* 初始化muart3 */
    //Bsp_UartMixed_Init(UART_MIXED, sensor_uart_rx_callback, sensor_uart_tx_callback);
    rt_kprintf("muart3 initialized!\n");
    
    /* 创建传感器数据发送线程 */
    thread_SensorData = rt_thread_create( "SensorData",             /* 线程名字 */
                                        SensorData_task,          /* 线程入口函数 */
                                        RT_NULL,                 /* 线程入口函数参数 */
                                        512,          		  /* 线程栈大小 */
                                        15,                      /* 线程的优先级 */
                                        20);                     /* 线程时间片 */
    if(thread_SensorData != RT_NULL)
    {
        rt_thread_startup(thread_SensorData);
        rt_kprintf("thread_SensorData startup!\n");
    }
    
    /* 创建ZDT电机指令接收线程 */
    thread_ZDTCommand = rt_thread_create( "CFFCommand",             /* 线程名字 */
                                         CFFCommand_task,          /* 线程入口函数 */
                                         RT_NULL,                 /* 线程入口函数参数 */
                                         512,          		  /* 线程栈大小 */
                                         14,                      /* 线程的优先级（比传感器数据线程高） */
                                         20);                     /* 线程时间片 */
    if(thread_ZDTCommand != RT_NULL)
    {
        rt_thread_startup(thread_ZDTCommand);
        rt_kprintf("thread_ZDTCommand startup!\n");
    }
		
    /* 创建机械臂电机指令接收线程 */
    thread_ARMCommand = rt_thread_create( "ARMCommand",             /* 线程名字 */
                                         Robot_arm_task,          /* 线程入口函数 */
                                         RT_NULL,                 /* 线程入口函数参数 */
                                         512,          		  /* 线程栈大小 */
                                         13,                      /* 线程的优先级 */
                                         20);                     /* 线程时间片 */
    if(thread_ARMCommand != RT_NULL)
    {
        rt_thread_startup(thread_ARMCommand);
        rt_kprintf("thread_ARMCommand startup!\n");
    }
    
    return 0;
}

INIT_APP_EXPORT(Task_Sensor_create);