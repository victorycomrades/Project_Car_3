/* Includes ------------------------------------------------------------------*/
#include "thread_comm.h"
#include "bsp.h"
#include "data.h"
#include "CheckCalc.h"

#include "thread_sensor.h"
/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* 定义线程控制块指针 */
rt_thread_t thread_comm = RT_NULL;

/* Private functions ---------------------------------------------------------*/
/**
 * @brief 解析结果变量，如需跨文件调用，需要外部声明
 */
float pos_x=0;
float pos_y=0;
float zangle=0;
int Wheel_isReady = 0;
int StartTemperature = 0;//开始校准温度
bool CalibratResults = false;//校准结果
/**
 * @brief 数据解析函数  如更换MCU平台或更换软件库，只需将串口接收到的值传入该函数即可解析
 * @param  rec 串口接收到的字节数据
 */
bool isDigit(char c)
{
    return (c >= '0' && c <= '9');
}
void Data_Analyse(uint8_t rec)//编码轮数据解析
{
	static uint8_t ch;
	static union
	{
		uint8_t date[24];
		float ActVal[6];
	}posture;
	static uint8_t count=0;
	static uint8_t i=0;

	ch=rec;
	switch(count)
	{
		case 0:
			if(ch==0x0d)
				count++;
			else
				count=0;
			break;
		case 1:
			if(ch==0x0a)
			{
				i=0;
				count++;
			}
			else if(ch==0x0d);
			else
				count=0;
			break;
		case 2:
			posture.date[i]=ch;
			i++;
			if(i>=24)
			{
				i=0;
				count++;
			}
			break;
		case 3:
			if(ch==0x0a)
				count++;
			else
				count=0;
			break;
		case 4:
			if(ch==0x0d)
			{
				zangle=posture.ActVal[0];
				pos_x=posture.ActVal[3];
				pos_y=posture.ActVal[4];
				if(Wheel_isReady == 0)
					{
						Wheel_isReady = 1;
					}
			}
			count=0;
			break;
		default:
			count=0;
		break;
	}
}
static void PCModule_Transmit(uint8_t *pData, uint16_t Size)//视觉主机串口发送
{
	Bsp_UARTMixed_TxTrigger(&muart2, (char*)pData, Size);
}

static void JY61_DataAnalysis(char *pdata, uint16_t len, uint8_t* pmark, int16_t* pstcAcc, int16_t* pstcGyro, int16_t* pstcAngle, int16_t* pstcTemp )
{
	uint8_t i;
	
	for(i=0; i<3; i++)
	{
		if( 0x55 == pdata[i*11] )
		{//帧头
			if( checksum((uint8_t*)&pdata[i*11],10) == pdata[i*11+10] )
			{//和校验
				switch(pdata[i*11+1])
				{
					case 0x51:
						pstcAcc[0] = pdata[i*11+2] | (pdata[i*11+3]<<8);
						pstcAcc[1] = pdata[i*11+4] | (pdata[i*11+5]<<8);
						pstcAcc[2] = pdata[i*11+6] | (pdata[i*11+7]<<8);
						*pstcTemp = pdata[i*11+8] | (pdata[i*11+9]<<8);
						*pmark |= 0x01;
						break;
					case 0x52:
						pstcGyro[0] = pdata[i*11+2] | (pdata[i*11+3]<<8);
						pstcGyro[1] = pdata[i*11+4] | (pdata[i*11+5]<<8);
						pstcGyro[2] = pdata[i*11+6] | (pdata[i*11+7]<<8);
						*pmark |= 0x02;
						break;
					case 0x53:
						pstcAngle[0] = pdata[i*11+2] | (pdata[i*11+3]<<8);
						pstcAngle[1] = pdata[i*11+4] | (pdata[i*11+5]<<8);
						pstcAngle[2] = pdata[i*11+6] | (pdata[i*11+7]<<8);
						*pmark |= 0x04;
						break;
				}
			}
		}
	}
}
static void HWT101_DataAnalysis(char *pdata, uint16_t len, uint8_t* pmark, int16_t* pstcGyro, int16_t* pstcAngle )
{
	uint8_t i;
	
	for(i=0; i<2; i++)
	{
		if( 0x55 == pdata[i*11] )
		{//帧头
			if( checksum((uint8_t*)&pdata[i*11],10) == pdata[i*11+10] )
			{//和校验
				switch(pdata[i*11+1])
				{
					case 0x52:
						*pstcGyro = pdata[i*11+6] | (pdata[i*11+7]<<8);
						*pmark |= 0x02;
						break;
					case 0x53:
						*pstcAngle = pdata[i*11+6] | (pdata[i*11+7]<<8);
						*pmark |= 0x04;
						break;
				}
			}
		}
	}
}

/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/***********************************************************************************************

                                       应用事件（中断）函数

************************************************************************************************/
/**
  * @brief  串口接收完成事件
  * @param  None
  * @retval None
  */


#if 0

void XferExternalUart2Rx_Handler(char *pdata, uint16_t len)//视觉
{
	uint8_t i;
	
	RecognitionModule_ReceivingProcess(&PCModule_t,(uint8_t*)pdata,len);
	if( PCModule_t.RecognitionModuleSte == RM_succeed )
	{//接收成功
		for(i=0; i<Recognition_RX_LEN; i++)
			Recognition_Buffer[i] = pdata[i];
		if(PCModule_t.state == RM_Colorstart)
		{
			if(pdata[0] == '1')
			    CurrentMaterialColor = RED;
			else if(pdata[0] == '2')
			    CurrentMaterialColor = GREEN;
			else if(pdata[0] == '3')
			    CurrentMaterialColor = BLUE;
		}
		if(PCModule_t.state == RM_Sequence)
		{

		}
		if(PCModule_t.state == RM_Location)
		{
			if( ( pdata[0] == '(' ) && ( pdata[len-2] == ')' ) )
			{
				cartesian_loc.x = atol(pdata+1);
				while( ( *pdata != ',' ) && ( *pdata != '\0' ) && ( *pdata != ')' ) )
					pdata++;
				cartesian_loc.y = atol(pdata+1);
			}
		}
		if(PCModule_t.state == RM_Circle)
		{
			if( ( pdata[0] == '(' ) && ( pdata[len-2] == ')' ) )
			{
				cartesian_loc.x = atol(pdata+1);
				while( ( *pdata != ',' ) && ( *pdata != '\0' ) && ( *pdata != ')' ) )
					pdata++;
				cartesian_loc.y = atol(pdata+1);
			}
		}
		bool_recognitionflag = 1;
	}
}


#else
void XferExternalUart2Rx_Handler(char *pdata, uint16_t len)//Nano
{
		sensor_uart_rx_callback(pdata,len);

}

#endif

#if 1
void XferExternalUart3Rx_Handler(char *pdata, uint16_t len)//陀螺仪
{//接收完成回调函数指针
	if( ( len == 33 ) || ( len == 22 )|| ( len == 11 ) )
		JY61_DataAnalysis( pdata, len, &GyroData_Struct.mark, GyroData_Struct.stcAcc, GyroData_Struct.stcGyro, GyroData_Struct.stcAngle, &GyroData_Struct.stcTemp );
	else if(len == 22)
		HWT101_DataAnalysis( pdata, len, &GyroData_Struct.mark, &GyroData_Struct.stcGyro[2], &GyroData_Struct.stcAngle[2] );
	
	if( ( GyroData_Struct.mark & 0x01 ) == 0x01 )
	{
		GyroData_Struct.temp = GyroData_Struct.stcTemp/340.0f+36.53f;
		GyroData_Struct.ax = (float)GyroData_Struct.stcAcc[0]*16.0f/32768.0f;//(g 为重力加速度，可取 9.8m/s 2 )
		GyroData_Struct.ay = (float)GyroData_Struct.stcAcc[1]*16.0f/32768.0f;//(g 为重力加速度，可取 9.8m/s 2 )
		GyroData_Struct.az = (float)GyroData_Struct.stcAcc[2]*16.0f/32768.0f;//(g 为重力加速度，可取 9.8m/s 2 )
		GyroData_Struct.mark &= (0xfe);
	}
	if( ( GyroData_Struct.mark & 0x02 ) == 0x02 )
	{
		GyroData_Struct.wx = (float)GyroData_Struct.stcGyro[0]*2000/32768.0f;
		GyroData_Struct.wy = (float)GyroData_Struct.stcGyro[1]*2000/32768.0f;
		GyroData_Struct.wz = (float)GyroData_Struct.stcGyro[2]*2000/32768.0f;
		GyroData_Struct.mark &= (0xfd);
	}
	if( ( GyroData_Struct.mark & 0x04 ) == 0x04 )
	{
		GyroData_Struct.Roll  = GyroData_Struct.stcAngle[0]*180/32768.0f;//滚转角（x 轴）
		GyroData_Struct.Pitch = GyroData_Struct.stcAngle[1]*180/32768.0f;//俯仰角（y 轴）
		GyroData_Struct.Yaw   = GyroData_Struct.stcAngle[2]*180/32768.0f;//偏航角（z 轴）
		GyroData_Struct.mark &= (0xfb);
	}
}

#else
static void RecognitionModule_Transmit(uint8_t *pData, uint16_t Size)//二维码模块串口发送
{
	Bsp_UARTMixed_TxTrigger(&muart3, (char*)pData, Size);
}
void XferExternalUart3Rx_Handler(char *pdata, uint16_t len)//二维码模块——TTL通信
{
	
	//接收完成回调函数指针
	uint8_t i;
	
	RecognitionModule_ReceivingProcess(&RecognitionModule_t,(uint8_t*)pdata,len);
	if( RecognitionModule_t.RecognitionModuleSte == RM_succeed )
	{//接收成功
		for(i=0; i<Recognition_RX_LEN; i++)
			Recognition_Buffer[i] = pdata[i];
		for(uint8_t j=0; j<2; j++)
		{
			for(uint8_t i=0; i<3; i++)  
			{//识别到的字符串对应颜色
				switch(pdata[(4*j)+i])
				{
					case '1':
						ColorSequence[j][i] = RED;
						break;
					case '2':
						ColorSequence[j][i] = GREEN;
						break;
					case '3':
						ColorSequence[j][i] = BLUE;
						break;
					default:break;
				}
			}
		}
		bool_recognitionflag = 1;
	}
}


#endif
void XferExternalUart4Rx_Handler(char *pdata, uint16_t len)//编码轮
{
		uint8_t i;
//  以下ACTION编码轮数据解析
		for(i=0; i<len; i++)
		{
			Data_Analyse(pdata[i]);
		}
    char *startPos = strstr(pdata, "start"); // 查找 "start" 的位置
    char *checkPos = strstr(pdata, "check"); // 查找 "check" 的位置
    if (startPos != NULL) 
		{
				char *numStart = startPos + strlen("start"); // 将指针移动到数字的起始位置
				int extractedValue = 0;

				// 遍历字符串，仅处理数字字符
				for (int i = 0; numStart[i] != '\0'; i++) {
						if (isDigit(numStart[i])) {
								// 如果是数字字符，将其转换为整数并添加到 extractedValue
								extractedValue = extractedValue * 10 + (numStart[i] - '0');
						} else {
								// 如果遇到非数字字符，停止提取
								break;
						}
				}
				StartTemperature = extractedValue;
		}
		else if(checkPos != NULL)
		{
			CalibratResults = 1;
		}
}
void XferExternalUart6Rx_Handler(char *pdata, uint16_t len)//二维码模块——TTL通信
{
	
	//接收完成回调函数指针
	uint8_t i;
	
	RecognitionModule_ReceivingProcess(&RecognitionModule_t,(uint8_t*)pdata,len);
	if( RecognitionModule_t.RecognitionModuleSte == RM_succeed )
	{//接收成功
		for(i=0; i<Recognition_RX_LEN; i++)
			Recognition_Buffer[i] = pdata[i];
		bool_recognitionflag = 1;
	}
}
/**
  * @brief  外部中断
  * @param  None
  * @retval None
  */
void EXTI_Callback(uint16_t GPIO_Pin) 
{
	switch(GPIO_Pin)
	{
		case ECHO1_Pin:
			HCSR04_PulseWidthRead(&sHCSR04Data[0]);
			break;
		case ECHO2_Pin:
			HCSR04_PulseWidthRead(&sHCSR04Data[1]);
			break;
		case ECHO3_Pin:
			HCSR04_PulseWidthRead(&sHCSR04Data[2]);
			break;
		case ECHO4_Pin:
			HCSR04_PulseWidthRead(&sHCSR04Data[3]);
			break;
	}
}
/**
  * @brief  定时中断事件
  * @param  None
  * @retval None
  */
static void TimeBreakExecution_Handler(void)
{//定时中断10us
	HCSR04Work_Scan(&sHCSR04Data[0]);
	HCSR04Work_Scan(&sHCSR04Data[1]);
	HCSR04Work_Scan(&sHCSR04Data[2]);
	HCSR04Work_Scan(&sHCSR04Data[3]);
}
static void Timer2UpdateBreakExecution_Handler(void)
{//TIM2溢出中断 2.73ms
	HCSR04_TimeOverflow(&sHCSR04Data[0]);
	HCSR04_TimeOverflow(&sHCSR04Data[1]);
	HCSR04_TimeOverflow(&sHCSR04Data[2]);
	HCSR04_TimeOverflow(&sHCSR04Data[3]);
}
/***********************************************************************************************

                                       应用任务函数

************************************************************************************************/
void comm_task(void *pvParameters)
{
#ifdef __SLAVE_SteeringEngine_6CH_H__
	uint8_t time = 0;
#endif
	while(1)
	{
	#ifdef __SLAVE_SteeringEngine_6CH_H__
		if(++time == 20)
		{
			time = 0;
			SLAVE_SteeringEngine6CH_MoreMotorControl( &Servo_S,
											          AngleValue[0], AngleTime[0],
											          AngleValue[1], AngleTime[1],
											          AngleValue[2], AngleTime[2],
											          AngleValue[3], AngleTime[3],
											          AngleValue[4], AngleTime[4],
											          AngleValue[5], AngleTime[5] );
		}
	 #endif
		//=========CAN通讯协议============================
		CANCommunication_Scan();            //最好也1ms进行扫描
		//=========识别模块通讯协议============================
		RecognitionModule_Scan1Ms(&RecognitionModule_t);
		RecognitionModule_Scan1Ms(&PCModule_t);
		//=========数据储存============================
		Application_DataFlash_Modification();
		//延时1MS
		My_mDelay( 0 );
	}
}
/***********************************************************************************************

                                       应用主函数

************************************************************************************************/
int Task_COMM_create(void)
{
	/*********************直流电机初始化***************************/
#ifdef __SLAVE_DCMotorMiniwatt_H__
	SLAVE_DCMotorMiniwatt_Init(&DCMotorMiniwatt1_S,1);//ID号1
	SLAVE_DCMotorMiniwatt_Init(&DCMotorMiniwatt2_S,2);//ID号2
	SLAVE_DCMotorMiniwatt_Init(&DCMotorMiniwatt3_S,3);//ID号3
	SLAVE_DCMotorMiniwatt_Init(&DCMotorMiniwatt4_S,4);//ID号4
#endif
	/*********************舵机初始化***************************/
#ifdef __SLAVE_SteeringEngine_3CH_H__
	SLAVE_SteeringEngine3CH_Init(&Servo_S,1);//舵机模块参数初始化
#endif
	/*********************巡线初始化***************************/
#ifdef __SLAVE_Tracking_H__
	SLAVE_Tracking_Init(&Tracking_Device1,1,0xff); //初始化巡线条模块,ID为1
	SLAVE_Tracking_Init(&Tracking_Device2,2,0xff); //初始化巡线条模块,ID为2
	SLAVE_Tracking_Init(&Tracking_Device3,3,0xff); //初始化巡线条模块,ID为3
	SLAVE_Tracking_Init(&Tracking_Device4,4,0xff); //初始化巡线条模块,ID为4
#endif
	/**********************CAN初始化***************************/
	/* CAN1 1000kbps * CAN2 500kbps */
	CANCommunication_Init();
//	SLAVE_SteeringEngine3CH_BUSMotorControl(&Servo_S,254,0,2250,50);	
	/**********************串口初始化***************************/
	Bsp_UartMixed_Init( &muart2, XferExternalUart2Rx_Handler, 0 );
	Bsp_UartMixed_Init( &muart3, XferExternalUart3Rx_Handler, 0 );
	Bsp_UartMixed_Init( &muart4, XferExternalUart4Rx_Handler, 0 );
	Bsp_UartMixed_Init( &muart6, XferExternalUart6Rx_Handler, 0 );
	/**********************识别模块通讯初始化**************************/  					
	RecognitionModule_Init(&PCModule_t,PCModule_Transmit);
	/*********************HCSR04初始化***************************/
	HCSR04_Inti( &sHCSR04Data[0],Echo1_read,Trig1_level,1 );     
	HCSR04_Inti( &sHCSR04Data[1],Echo2_read,Trig2_level,1 );   
	HCSR04_Inti( &sHCSR04Data[2],Echo3_read,Trig3_level,1 );     
	HCSR04_Inti( &sHCSR04Data[3],Echo4_read,Trig4_level,1 ); 	
	/********************TIME初始化**************************/
	EXTI_SetHandler(EXTI_Callback) ;
	Timer_SetHandler(TimeBreakExecution_Handler);
	Timer2Update_SetHandler(Timer2UpdateBreakExecution_Handler);
	/**********************创建任务****************************/
	thread_comm = rt_thread_create("comm",           /* 线程名字 */
								 comm_task,        /* 线程入口函数 */
								 RT_NULL,          /* 线程入口函数参数 */
								 1024,              /* 线程栈大小 */
								 1,                /* 线程的优先级 */
								 20);              /* 线程时间片 */
	if(thread_comm != RT_NULL)
	{
		rt_thread_startup(thread_comm);
		rt_kprintf("thread_comm startup!\n");
	}
	
	return 0;
}
INIT_APP_EXPORT(Task_COMM_create);

