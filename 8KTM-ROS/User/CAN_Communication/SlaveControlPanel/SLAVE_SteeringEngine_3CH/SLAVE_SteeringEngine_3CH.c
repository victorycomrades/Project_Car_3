/* 包含头文件 ----------------------------------------------------------------*/
#include "SLAVE_SteeringEngine_3CH/SLAVE_SteeringEngine_3CH.h"
#include "string.h"
#include "data.h"
#include "CAN_Communication.h"
/* 私有类型定义 --------------------------------------------------------------*/

/* 私有宏定义 ----------------------------------------------------------------*/

/* 私有变量 ------------------------------------------------------------------*/
static SteeringEngine3CHDef_t* head_handle = NULL;
/* 扩展变量 ------------------------------------------------------------------*/

/* 私有函数原形 --------------------------------------------------------------*/

/* 函数体 --------------------------------------------------------------------*/
static uint16_t SLAVE_SteeringEngine_3CH_ProtocolDefinitions(SteeringEngine3CHDef_t* handle,
															 uint16_t cannum,
															 PgnCanTxDataType *head)
{
    uint16_t cannum_buff;
    cannum_buff = cannum;
    
    handle->CAN_Number = cannum_buff; //第一个帧的CAN编号，后面以次累加计算
    
    head[cannum_buff++] = xCreate( DEVICE_ID,DEVICE_LIST,
								   SteeringEngine3CHPlate_Addr,
								   handle->SteeringEngine_SetPara.DATE.Serial_Number,
								   2,2,
                                   TRUE,PG_TX_FREE,MODE_REQ,TT_ASYNC,10,0,  
                                   handle->SteeringEngine_SetPara.Datum,PG_RXSTE_PARA_DATA_LEN,
								   handle->SteeringEngine_SetPara.Datum,PG_RXSTE_PARA_DATA_LEN); //参数修改 
	
    head[cannum_buff - 1].ch = 1;    
				
    head[cannum_buff++] = xCreate( DEVICE_ID,DEVICE_LIST,
	                               SteeringEngine3CHPlate_Addr,
								   handle->SteeringEngine_SetPara.DATE.Serial_Number,
								   3,3,   
                                   TRUE,PG_TX_FREE,MODE_REQ,TT_ASYNC,10,0,    
                                   handle->SteeringBusServo_ControlData.Datum,PG_RXSTE_BusServoCtrl_DATA_LEN,
								   handle->SteeringBusServo_ControlData.Datum,PG_RXSTE_BusServoCtrl_DATA_LEN);
									 
		head[cannum_buff - 1].ch = 1;    		
		
    head[cannum_buff++] = xCreate( DEVICE_ID,DEVICE_LIST,
	                               SteeringEngine3CHPlate_Addr,
								   handle->SteeringEngine_SetPara.DATE.Serial_Number,
								   4,4,   
                                   TRUE,PG_TX_FREE,MODE_REQ,TT_ASYNC,10,0,    
                                   handle->SteeringBusServo_SetData.Datum,PG_RXSTE_BusServoSet_DATA_LEN,
								   handle->SteeringBusServo_SetData.Datum,PG_RXSTE_BusServoSet_DATA_LEN);

		head[cannum_buff - 1].ch = 1;    

    head[cannum_buff++] = xCreate( DEVICE_ID,DEVICE_LIST,
	                               SteeringEngine3CHPlate_Addr,
								   handle->SteeringEngine_SetPara.DATE.Serial_Number,
								   5,5,   
                                   TRUE,PG_TX_FREE,MODE_BRO,TT_CYCLIC,10,0,    
                                   0,0,
								   handle->SteeringBusServo_GetData[0].Datum,PG_RXSTE_BusServoSet_DATA_LEN);

		head[cannum_buff - 1].ch = 1;    
		
    head[cannum_buff++] = xCreate( DEVICE_ID,DEVICE_LIST,
	                               SteeringEngine3CHPlate_Addr,
								   handle->SteeringEngine_SetPara.DATE.Serial_Number,
								   6,6,   
                                   TRUE,PG_TX_FREE,MODE_BRO,TT_CYCLIC,10,0,    
                                   0,0,
								   handle->SteeringBusServo_GetData[1].Datum,PG_RXSTE_BusServoSet_DATA_LEN);

		head[cannum_buff - 1].ch = 1;    
    return cannum_buff;
}

uint16_t SLAVE_SteeringEngine3CH_CANinit(uint16_t cannum,void *head)
{
	SteeringEngine3CHDef_t* target;
	uint16_t cannum_buff;

	cannum_buff = cannum;
	for(target=head_handle; target; target=target->next) 
	{
		cannum_buff = SLAVE_SteeringEngine_3CH_ProtocolDefinitions(target,cannum_buff,((PgnCanTxDataType*)head));
	}
   
	return cannum_buff;
}

uint8_t SLAVE_SteeringEngine3CH_Init(SteeringEngine3CHDef_t* handle,uint8_t list)
{
	SteeringEngine3CHDef_t* target = head_handle;
    
	memset(handle, 0, sizeof(SteeringEngine3CHDef_t));
	
	handle->SteeringEngine_SetPara.DATE.Serial_Number = list;
    
	while(target) 
	{
		if(target == handle) 
            return 0;	//already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
    
	return 1;
}
void SLAVE_SteeringEngine3CH_NumberSet(SteeringEngine3CHDef_t* handle,uint8_t number)
{
	if(number > 15)
	{
		number = 15;
	}
    
	handle->SteeringEngine_SetPara.DATE.Serial_Number = number;
	handle->SteeringEngine_SetPara.DATE.mode.bit.modifyidflg = 1;

	CAN_TT_ASYNC_Enable(handle->CAN_Number);
}
void SLAVE_SteeringEngine3CH_MovementMemory(SteeringEngine3CHDef_t* handle)
{
	handle->SteeringEngine_SetPara.DATE.mode.bit.memoryflg = 1;
	CAN_TT_ASYNC_Enable(handle->CAN_Number);
}
void SLAVE_SteeringEngine3CH_FullActRun(SteeringEngine3CHDef_t* handle,uint8_t actFullnum)
{
	handle->SteeringEngine_SetPara.DATE.mode.bit.actFullRunflg = 1;
	handle->SteeringEngine_SetPara.DATE.actFullnum = actFullnum;
	CAN_TT_ASYNC_Enable(handle->CAN_Number);
}
void SLAVE_SteeringEngine3CH_FullActStop(SteeringEngine3CHDef_t* handle)
{
	handle->SteeringEngine_SetPara.DATE.mode.bit.actFullStopflg = 1;
	CAN_TT_ASYNC_Enable(handle->CAN_Number);
}

void SLAVE_SteeringEngine3CH_BUSMotorControl(SteeringEngine3CHDef_t* handle,
                                           uint8_t ID,int16_t Position,int16_t Speed,uint16_t ACC)
{
	if(ID >= 254)
	{
		ID = 254;
	}

	if(ACC < 10)
	{
		ACC = 10;
	}
	if(ACC > Speed)
	{
		ACC = Speed;
	}
	if(Speed<0)
	{
		handle->SteeringBusServo_SetData.DATE.Speed = (-1*Speed) | 0x8000;
	}
	else
	{
		handle->SteeringBusServo_SetData.DATE.Speed = Speed;	
	}
	handle->SteeringBusServo_SetData.DATE.ID = ID;
	handle->SteeringBusServo_SetData.DATE.Position = Position;

	handle->SteeringBusServo_SetData.DATE.ACC = ACC;	
	
	CAN_TT_ASYNC_Enable(handle->CAN_Number+2);
	
	HAL_Delay(5);
}


int16_t SLAVE_SteeringEngine3CH_BUSMotorDATAControl(SteeringEngine3CHDef_t* handle,uint8_t a,uint8_t b)
{
	if(b == 0)
	{
		return handle->SteeringBusServo_GetData[a].DATE.Position1;		
	}
	else if(b == 1)
	{
		return handle->SteeringBusServo_GetData[a].DATE.Position2;		
	}
	else if(b == 2)
	{
		return handle->SteeringBusServo_GetData[a].DATE.Position3;		
	}
	else if(b == 3)
	{
		return handle->SteeringBusServo_GetData[a].DATE.Position4;		
	}
}



