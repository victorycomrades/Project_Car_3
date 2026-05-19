#ifndef __SLAVE_SteeringEngine_3CH_H__
#define __SLAVE_SteeringEngine_3CH_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stdint.h"

/* 类型定义 ------------------------------------------------------------------*/
typedef struct SteeringEngine_3CH_T
{      
	uint8_t  CAN_Number;

	union 
	{
		#define PG_RXSTE_PARA_DATA_LEN      8
		uint8_t Datum[PG_RXSTE_PARA_DATA_LEN];
		struct
		{
			union
			{
				uint8_t byte;
				#define SETPARA_ID_FLG          0x01
				#define SETPARA_MEMORY_FLG      0x02
				#define SETPARA_ACTFULLRUN_FLG  0x04
				#define SETPARA_ACTFULLSTOP_FLG 0x08
				struct{
					uint8_t modifyidflg    :1;
					uint8_t memoryflg      :1;
					uint8_t actFullRunflg  :1;
					uint8_t actFullStopflg :1;
					uint8_t retain         :4;
				}bit;
			}mode;
			uint8_t Serial_Number;
			uint8_t actFullnum;
			uint8_t alter_mode;
			uint8_t retain2;
			uint8_t retain3;
			uint8_t retain4;
			uint8_t retain5;
			}DATE;
	}SteeringEngine_SetPara;
  
	union 
	{
		#define PG_RXSTE_BusServoCtrl_DATA_LEN 8
		uint8_t Datum[PG_RXSTE_BusServoCtrl_DATA_LEN];
		struct{
			uint8_t ID;
			uint8_t MemAddr;
			uint8_t len;
			uint8_t function;
			uint8_t data[4];	
		}DATE;
	}SteeringBusServo_ControlData;
	
	union 
	{
		#define PG_RXSTE_BusServoSet_DATA_LEN 8
		uint8_t Datum[PG_RXSTE_BusServoSet_DATA_LEN];
		struct{
			int16_t ID;
			int16_t Position;
			int16_t Speed;
			int16_t ACC;		
		}DATE;
	}SteeringBusServo_SetData;

	union 
	{
		#define PG_RXSTE_BusServoGet_DATA_LEN 8
		uint8_t Datum[PG_RXSTE_BusServoGet_DATA_LEN];
		struct{
			int16_t Position1;
			int16_t Position2;
			int16_t Position3;
			int16_t Position4;	
		}DATE;
	}SteeringBusServo_GetData[2];
	
	struct SteeringEngine_3CH_T* next;
} SteeringEngine3CHDef_t;
/* 宏定义 --------------------------------------------------------------------*/
#define SLAVE_SteeringEngine3CH_ENABLED 1
#define SteeringEngine3CHPlate_Addr     4  //舵机板地址

//内存表定义
//-------EPROM(只读)--------
#define SMS_STS_MODEL_L 3
#define SMS_STS_MODEL_H 4

//-------EPROM(读写)--------
#define SMS_STS_ID 5
#define SMS_STS_BAUD_RATE 6
#define SMS_STS_MIN_ANGLE_LIMIT_L 9
#define SMS_STS_MIN_ANGLE_LIMIT_H 10
#define SMS_STS_MAX_ANGLE_LIMIT_L 11
#define SMS_STS_MAX_ANGLE_LIMIT_H 12
#define SMS_STS_CW_DEAD 26
#define SMS_STS_CCW_DEAD 27
#define SMS_STS_OFS_L 31
#define SMS_STS_OFS_H 32
#define SMS_STS_MODE 33

//-------SRAM(读写)--------
#define SMS_STS_TORQUE_ENABLE 40
#define SMS_STS_ACC 41
#define SMS_STS_GOAL_POSITION_L 42
#define SMS_STS_GOAL_POSITION_H 43
#define SMS_STS_GOAL_TIME_L 44
#define SMS_STS_GOAL_TIME_H 45
#define SMS_STS_GOAL_SPEED_L 46
#define SMS_STS_GOAL_SPEED_H 47
#define SMS_STS_LOCK 55

//-------SRAM(只读)--------
#define SMS_STS_PRESENT_POSITION_L 56
#define SMS_STS_PRESENT_POSITION_H 57
#define SMS_STS_PRESENT_SPEED_L 58
#define SMS_STS_PRESENT_SPEED_H 59
#define SMS_STS_PRESENT_LOAD_L 60
#define SMS_STS_PRESENT_LOAD_H 61
#define SMS_STS_PRESENT_VOLTAGE 62
#define SMS_STS_PRESENT_TEMPERATURE 63
#define SMS_STS_MOVING 66
#define SMS_STS_PRESENT_CURRENT_L 69
#define SMS_STS_PRESENT_CURRENT_H 70

//功能表定义
#define BUS_Write_Gen 0x00
#define BUS_Write_Reg 0x02
#define BUS_Read_Reg  0x05

/* 扩展变量 ------------------------------------------------------------------*/

/* 函数声明 ------------------------------------------------------------------*/

uint8_t SLAVE_SteeringEngine3CH_Init(SteeringEngine3CHDef_t* handle,uint8_t list);
void SLAVE_SteeringEngine3CH_NumberSet(SteeringEngine3CHDef_t* handle,uint8_t number);
void SLAVE_SteeringEngine3CH_FullActRun(SteeringEngine3CHDef_t* handle,uint8_t actFullnum);
void SLAVE_SteeringEngine3CH_FullActStop(SteeringEngine3CHDef_t* handle);
void SLAVE_SteeringEngine3CH_MotorControl(SteeringEngine3CHDef_t* handle,
                                       uint8_t  Channel,
                                       uint16_t ServoPwmDutySetCh,
									   uint16_t time);
void SLAVE_SteeringEngine3CH_MoreMotorControl(SteeringEngine3CHDef_t* handle,
                                           uint16_t ServoPwmDutySetCh1,uint16_t time1,
									       uint16_t ServoPwmDutySetCh2,uint16_t time2,
                                           uint16_t ServoPwmDutySetCh3,uint16_t time3);
void SLAVE_SteeringEngine3CH_MovementMemory(SteeringEngine3CHDef_t* handle);
void SLAVE_SteeringEngine3CH_BUSMotorControl(SteeringEngine3CHDef_t* handle,
                                           uint8_t ID,int16_t Position,int16_t Speed,uint16_t ACC);
int16_t SLAVE_SteeringEngine3CH_BUSMotorDATAControl(SteeringEngine3CHDef_t* handle,uint8_t a,uint8_t b);

#endif  // __SLAVE_SteeringEngine_6CH_H__

