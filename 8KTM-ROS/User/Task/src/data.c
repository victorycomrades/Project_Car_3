/* Includes ------------------------------------------------------------------*/
#include "data.h"
#include "CheckCalc.h"
#include <rthw.h>
/* Private macros ------------------------------------------------------------*/
#define VER_LEN                20  //版本字节长度
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
const FlashData_Typedef FactoryData_S =
{
	.FH1                      = 0x5A,
	.FH2                      = 0xA5,
    .Servo_RotationClawInit   = 1420,
	.Servo_Claw_S             = 1750,
	.Servo_Claw_J             = 1300,
	.FlashData_Len            = sizeof( FlashData_Typedef ), //存储数据的长度
	.Bit16CRC                 = 0x4203,
};
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
FlashData_Typedef FlashData_Struct;
AppData_Typedef   AppData_Struct;
/*******直流电机驱动模块数据声明*******/
#ifdef __SLAVE_DCMotorMiniwatt_H__
DCMotorMiniwattDef_t DCMotorMiniwatt_S[4];
#endif
/*******舵机驱动模块数据声明*******/
#ifdef __SLAVE_SteeringEngine_3CH_H__
SteeringEngine3CHDef_t Servo_S;
uint16_t AngleValue[6] = {1500,1500,1500,1500,1500,1500};
uint16_t AngleTime[6]  = {20,20,20,20,20,20};
#endif
/*******巡线条模块数据声明*******/
#ifdef __SLAVE_Tracking_H__
TrackingDef_t Tracking_Device[4];
#endif
/*******超声波模块数据声明*******/
#ifdef __SLAVE_UltrasonicRanging_H__
UltrasonicRangingDef_t UltrasonicRanging_S;
#endif
/*******陀螺仪数据声明*******/
GyroData_Typedef GyroData_Struct = {0};
/*******遥控数据声明*******/
_s_RemoteCtrl_Info sRemoteCtrl_Info = {0};
/*******识别模块数据声明*******/
RecognitionModule_s RecognitionModule_t;
/*******二维码颜色、圆位置模块数据声明*******/
RecognitionModule_s PCModule_t;
RM_cartesian cartesian_loc;
/*******颜色缓存数据声明*******/
uint8_t bool_recognitionflag = 0; 
uint8_t Recognition_Buffer[Recognition_RX_LEN] = {0};
uint16_t ColorSequence[2][3] = {RED,GREEN,BLUE,BLUE,GREEN,RED};
uint16_t CircleSequence[3] = {RED,GREEN,BLUE};
uint8_t bool_colorsequenceflag = 0;
uint8_t bool_circlesequenceflag = 0;
uint16_t CurrentMaterialColor = WHITE;

/*******砖数据声明*******/
BrickData_TypeDef BrickData_Struct = {0};
/* Exported functions --------------------------------------------------------*/
/**
  * @brief  修改过的数据存flash
  * @param  None
  * @retval 1：成功  0：失败
  */
uint8_t Application_DataFlash_Modification(void)
{
	uint8_t succeed = 0;
	
	if(DataStorageFlg_Start == AppData_Struct.DataStorage_Flg)
    {
		AppData_Struct.DataStorage_Flg = DataStorageFlg_Leisure;
		FlashData_Struct.FlashData_Len = FactoryData_S.FlashData_Len;
		rt_base_t level;
        level = rt_hw_interrupt_disable();
		FlashData_Struct.Bit16CRC = CRC16Modbus_Compute(&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len-2);
		stmflash_write(STM_FLASH_BASE, (uint32_t *)&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len/4);
		rt_hw_interrupt_enable(level);
	}
	else if(AppData_Struct.DataStorage_Flg > DataStorageFlg_ERRADDR)
    {//值为非法值
        AppData_Struct.DataStorage_Flg = DataStorageFlg_Leisure;
    }
	
	return succeed;
}
int AppData_Init(void)
{
	uint16_t i;
	
	/****************第一次存入flash**********************/
	for(i=0; i<(FactoryData_S.FlashData_Len/4); i++)
    {
        if(0xFFFFFFFF != ((uint32_t*)STM_FLASH_BASE)[i])
        {
            break;
        }
    }
	if(i >= (FactoryData_S.FlashData_Len/4))
    {
		FlashData_Struct = FactoryData_S;
		FlashData_Struct.Bit16CRC = CRC16Modbus_Compute(&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len-2);
		stmflash_write(STM_FLASH_BASE, (uint32_t *)&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len/4);
	}
	/****************读取数据**********************/
	FlashData_Struct = *((FlashData_Typedef*)STM_FLASH_BASE);
	i = CRC16Modbus_Compute(&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len-2);
	if( ( FlashData_Struct.FH1 != 0x5A ) \
	 || ( FlashData_Struct.FH2 != 0xA5 ) \
	 || ( i != FlashData_Struct.Bit16CRC ) )
	{//数据错误
		FlashData_Struct = FactoryData_S;
	    FlashData_Struct.Bit16CRC = CRC16Modbus_Compute(&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len-2);
		stmflash_write(STM_FLASH_BASE, (uint32_t *)&FlashData_Struct.FH1, FlashData_Struct.FlashData_Len/4);
	}
	
	return 0;
}
INIT_BOARD_EXPORT(AppData_Init);













