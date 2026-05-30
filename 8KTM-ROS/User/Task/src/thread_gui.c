/* Includes ------------------------------------------------------------------*/
#include "thread_gui.h"
#include "thread_comm.h"
#include "bsp.h"
#include "data.h"
#include "RGB.h"
#include "separate_button.h"
#include "thread_rccu.h"
#include "lcd.h"
#include "thread_sensor.h"
#include <stdio.h>
#include <finsh.h>
/* Private macros ------------------------------------------------------------*/
#define MOTOR_NUM_MAX          4
#define TRACKINGSENSOR_NUM_MAX 4
  
#define WINDOW_NUM             8 //液晶屏显示页数
/* Private types -------------------------------------------------------------*/
typedef enum
{
	DisplayAdd = 0,
	DisplayMinus = 1,
}_e_DisplayOperate;
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* 定义线程控制块指针 */
rt_thread_t thread_gui = RT_NULL;
rt_thread_t thread_bus = RT_NULL;
/*******独立按键数据声明*******/
KEY_T key1_struct;
KEY_T key2_struct;
KEY_T key3_struct;
KEY_T key4_struct;
KEY_T key5_struct;
uint8_t Debug_flag = 1; //坐标系显示
/*******直流电机驱动模块数据声明*******/
int16_t SpeedValue[MOTOR_NUM_MAX] = {0,0,0,0};
int16_t SpeedValuebuff[MOTOR_NUM_MAX] = {0xffff,0xffff,0xffff,0xffff};
int16_t ReadSpeedValue[MOTOR_NUM_MAX] = {0,0,0,0};
int16_t ReadSpeedValuebuff[MOTOR_NUM_MAX] = {0xffff,0xffff,0xffff,0xffff};
/*******巡线条模块数据声明*******/
uint8_t TrackingState[TRACKINGSENSOR_NUM_MAX] = {TRACKING_MODE_WORK,TRACKING_MODE_WORK,TRACKING_MODE_WORK,TRACKING_MODE_WORK};
uint8_t TrackingValue[TRACKINGSENSOR_NUM_MAX] = {0,0,0,0};
uint8_t TrackingValuebuff[TRACKINGSENSOR_NUM_MAX] = {0xff,0xff,0xff,0xff};
/*******超声波模块数据声明*******/
uint16_t UltrasonicRangingValuebuff[4] = {0xffff,0xffff,0xffff,0xffff};;
/*******MPU6050数据声明*******/
uint16_t stcAngleBuff[3] = {0xffff,0xffff,0xffff};//角度
/*******舵机数据声明*******/
int16_t BUS_Position[6] = {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff};
//lcd
uint8_t lcd_page = 0;
uint8_t lcd_line = 0;
uint8_t lcd_linebuff = 0xff;
static uint8_t _lcd_show_timeout = 0;
uint8_t window_flg = 0;
VoidFuncVoid LCD_Display_Hand = 0;
/*******时间数据声明*******/
uint32_t Time_Cnt = 0;
uint32_t Time_buff = 0;
/*******液晶函数预定义*******/
static void Display_Window_Clear(void);
static void Display_Window_StartPage(void);
static void Display_Window_DCBrushMotor(void);
static void Display_Window_EncoderCalibrating(void);
static void Display_Window_TrackingSensorBool(void);
static void Display_Window_UltrasonicSensor(void);
static void Display_Window_JY62_MPU6050Sensor(void);
static void Display_Window_BUS_ServoSensor(void);
/*******液晶显示变量*******/
const VoidFuncVoid DisplayWindow[WINDOW_NUM]=
{
	Display_Window_Clear,
	Display_Window_StartPage,
	Display_Window_DCBrushMotor,
	Display_Window_EncoderCalibrating,
	Display_Window_TrackingSensorBool,
	Display_Window_UltrasonicSensor,
	Display_Window_JY62_MPU6050Sensor,
	Display_Window_BUS_ServoSensor,
};
const uint8_t display[][14]=
{
	{"Motor1SetSpd:"},  //0
	{"Motor2SetSpd:"},  //1
	{"Motor3SetSpd:"},  //2
	{"Motor4SetSpd:"},  //3
	{"RSpd1:"},  //4
	{"RSpd2:"},  //5
	{"RSpd3:"},  //6
	{"RSpd4:"},  //7
	{"Tracking1:"},  //8
	{"Tracking2:"},  //9
	{"Tracking3:"},  //10
	{"Tracking4:"},  //11
	{"Rotary Init:"},//12
};
/* Private functions ---------------------------------------------------------*/
static void Display_Tick(uint8_t Tick)
{
	Time_Cnt += Tick; 
}
static void Display_123(uint16_t (*qdata)[3])//显示二维码
{
	

	uint8_t i,j;
	
	for(j=0;j<2;j++)
	{
		for(i=0;i<3;i++)
		{
			if(qdata[j][i] == RED)
				lcd_show_num(i*100, j*175, 64, 1, 1, 1);
			else if(qdata[j][i] == GREEN)
				lcd_show_num(i*100, j*175, 64, 2, 1, 1);
			else if(qdata[j][i] == BLUE)
				lcd_show_num(i*100, j*175, 64, 3, 1, 1);
		}
	}
	lcd_show_string(100, 90, 64, "+");
}
/*******电机控制*******/
static void DCMotor_Ctrl_Scan(void)
{
	if(LCD_Display_Hand == Display_Window_DCBrushMotor)
	{
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt1_S,SpeedValue[0]);
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt2_S,SpeedValue[1]);
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt3_S,SpeedValue[2]);
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt4_S,SpeedValue[3]);
	}
	ReadSpeedValue[0] = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt1_S);
	ReadSpeedValue[1] = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt2_S);
	ReadSpeedValue[2] = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt3_S);
	ReadSpeedValue[3] = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt4_S);
}
/*******循迹传感器读取*******/
static void TrackingSensor_Ctrl_Scan(void)
{ 
	TrackingValue[0] = SLAVE_Tracking_BoolRead(&Tracking_Device1);
	TrackingValue[1] = SLAVE_Tracking_BoolRead(&Tracking_Device2);
	TrackingValue[2] = SLAVE_Tracking_BoolRead(&Tracking_Device3);
	TrackingValue[3] = SLAVE_Tracking_BoolRead(&Tracking_Device4);
	TrackingState[0] = SLAVE_Tracking_ModeRead(&Tracking_Device1);
	TrackingState[1] = SLAVE_Tracking_ModeRead(&Tracking_Device2);
	TrackingState[2] = SLAVE_Tracking_ModeRead(&Tracking_Device3);
	TrackingState[3] = SLAVE_Tracking_ModeRead(&Tracking_Device4);
}
/*******总线舵机步数读取*******/
void BUSPosition_Ctrl_Scan(void)
{ 
	BUS_Position[0] = SLAVE_SteeringEngine3CH_BUSMotorDATAControl(&Servo_S,0,0);
	BUS_Position[1] = SLAVE_SteeringEngine3CH_BUSMotorDATAControl(&Servo_S,0,1);
	BUS_Position[2] = SLAVE_SteeringEngine3CH_BUSMotorDATAControl(&Servo_S,0,2);
	BUS_Position[3] = SLAVE_SteeringEngine3CH_BUSMotorDATAControl(&Servo_S,0,3);
	BUS_Position[4] = SLAVE_SteeringEngine3CH_BUSMotorDATAControl(&Servo_S,1,0);
	BUS_Position[5] = SLAVE_SteeringEngine3CH_BUSMotorDATAControl(&Servo_S,1,1);
}
static void Display_Window_Clear(void)
{
	if(0 == window_flg)
	{
		lcd_clear(BLUE);//清屏
		lcd_line = 0;//行到初始值
		lcd_linebuff = 0xff; 
		if(lcd_page == 0)
		{//如果是清屏页转换到第一数据页
			lcd_page = 1;
		}		
		LCD_Display_Hand = DisplayWindow[lcd_page];
		if(LCD_Display_Hand == Display_Window_DCBrushMotor)
		{//直流数据显示缓存清除
			SpeedValuebuff[0] = 0xffff;
			SpeedValuebuff[1] = 0xffff;
			SpeedValuebuff[2] = 0xffff;
			SpeedValuebuff[3] = 0xffff;
			ReadSpeedValuebuff[0] = 0xffff;
			ReadSpeedValuebuff[1] = 0xffff;
			ReadSpeedValuebuff[2] = 0xffff;
			ReadSpeedValuebuff[3] = 0xffff;
		}
		else if(LCD_Display_Hand == Display_Window_EncoderCalibrating)
		{
			lcd_show_string(6,5, 		12, "[EncoderCalibrating]");
			lcd_show_string(2,5+32, 12, "Temperature:");
			lcd_show_string(2,5+48, 12, "Results:");
			lcd_show_string(16,5+64+32, 12, "Double Click K1");
			stcAngleBuff[0] = 0xffff;
			stcAngleBuff[1] = 0xffff;
			stcAngleBuff[2] = 0xffff;
		}
		else if(LCD_Display_Hand == Display_Window_TrackingSensorBool)
		{//循迹数据显示缓存清除
			TrackingValuebuff[0] = 0x5a;
			TrackingValuebuff[1] = 0x5a;
			TrackingValuebuff[2] = 0x5a;
			TrackingValuebuff[3] = 0x5a;
		}
		else if(LCD_Display_Hand == Display_Window_UltrasonicSensor)
		{
			lcd_show_string(2+3*8,2+8, 12,"Ultrasonic");
			lcd_show_string(2+5*8,2+24, 12, "Signal");
			lcd_show_string(2, 2+8*8, 12, "DATA1:");
			lcd_show_string(66, 2+8*8, 12, "DATA2:");
			lcd_show_string(2, 2+8*11, 12, "DATA3:");
			lcd_show_string(66, 2+8*11, 12, "DATA4:");
//			lcd_show_string(2, 2+8*14, 12, "DATA5:");
//			lcd_show_string(66, 2+8*14, 12, "DATA6:");
			Time_buff = Time_Cnt-100;
			UltrasonicRangingValuebuff[0] = 0xffff;
			UltrasonicRangingValuebuff[1] = 0xffff;
			UltrasonicRangingValuebuff[2] = 0xffff;
			UltrasonicRangingValuebuff[3] = 0xffff;
		}
		else if(LCD_Display_Hand == Display_Window_JY62_MPU6050Sensor)
		{
			lcd_show_string(40,5, 12, "MPU6050");
			lcd_show_string(2,5+16, 12, "Roll :    . C");
			lcd_show_string(2,5+32, 12, "Pitch:    . C");
			lcd_show_string(2,5+48, 12, "Yaw  :    . C");
			stcAngleBuff[0] = 0xffff;
			stcAngleBuff[1] = 0xffff;
			stcAngleBuff[2] = 0xffff;
		}
		else if(LCD_Display_Hand == Display_Window_BUS_ServoSensor)
		{
			lcd_show_string(2, 2+0*8,  12, "BUS1Position:");
			lcd_show_string(2, 2+2*8,  12, "BUS2Position:");
			lcd_show_string(2, 2+4*8,  12, "BUS3Position:");
			lcd_show_string(2, 2+6*8,  12, "BUS4Position:");
			lcd_show_string(2, 2+8*8,  12, "BUS5Position:");
			lcd_show_string(2, 2+10*8, 12, "BUS6Position:");
			BUS_Position[0] = 0xffff;
			BUS_Position[1] = 0xffff;			
			BUS_Position[2] = 0xffff;			
			BUS_Position[3] = 0xffff;			
			BUS_Position[4] = 0xffff;			
			BUS_Position[5] = 0xffff;		
		}
		window_flg = 1;
	}
}
static void Display_Window_StartPage(void)
{
    static uint16_t time_cnt = 0;
    static uint16_t _task_disp_cnt = 0;

    /* task code display: persist for 200 cycles */
    if ( _lcd_ready == 1 )
    {
        _lcd_ready = 0;
        {
            uint8_t bi = 0, ci = 0;
            char *t = _lcd_text;
            while (*t && bi < 2) {
                if (*t >= '1' && *t <= '4') {
                    if (*t == '1') ColorSequence[bi][ci] = RED;
                    else if (*t == '2') ColorSequence[bi][ci] = GREEN;
                    else if (*t == '3') ColorSequence[bi][ci] = BLUE;
                    else if (*t == '4') ColorSequence[bi][ci] = RED;
                    ci++;
                    if (ci >= 3) { ci = 0; bi++; }
                }
                t++;
            }
        }
        _task_disp_cnt = 200;
    }

    if ( _task_disp_cnt > 0 )
    {
        _task_disp_cnt--;
        lcd_clear(BACK_COLOR);
        Display_123(ColorSequence);
        return;
    }

    if(lcd_linebuff != lcd_line)
    {
        lcd_linebuff = lcd_line;
        time_cnt = 0;
    }
    time_cnt++;
    if(time_cnt == 1)
        lcd_show_snum(10,2,12,BrickData_Struct.x,5,0);
    else if(time_cnt == 7)
        lcd_show_snum(10,18,12,BrickData_Struct.y,5,0);
    else if(time_cnt == 14)
        lcd_show_snum(10,34,12,BrickData_Struct.yaw,5,0);
    else if(time_cnt == 21)
    {
        lcd_show_snum(10,50,12,Read_Position_x_mm(),5,0);
        lcd_show_snum(10,66,12,Read_Position_y_mm(),5,0);
        time_cnt = 0;
    }

    if( bool_colorsequenceflag == 1 )
    {
        Debug_flag = 0;
        bool_colorsequenceflag = 0;
        lcd_clear(BACK_COLOR);
        Display_123(ColorSequence);
        time_cnt = 31;
    }

//	#ifdef EncodingWheel
    if(Wheel_isReady == 1)
    {
        buzzerSound(H6);
        My_mDelay(120);
        buzzerSound(H5);
        My_mDelay(100);
        buzzerSound(H4);
        My_mDelay(120);
        buzzerSound(0);
        Wheel_isReady = 2;
        lcd_clear(BACK_COLOR);
        lcd_show_string(12, 120,12,"Click K5 to Start");
    }
}
			
//			if(Wheel_isReady == 2)
//			{
//				if(Debug_flag == 1)//显示坐标系
//				{
//					char buffer_x[11];
//					char buffer_y[11];
//					char buffer_z[11];
//					snprintf(buffer_x, sizeof(buffer_x), "X:%.6f", pos_x); 
//					snprintf(buffer_y, sizeof(buffer_y), "Y:%.6f", pos_y);
//					snprintf(buffer_z, sizeof(buffer_z), "Z:%.6f", Read_Position_yaw());

//					if(time_cnt >= 20)
//					{
//						time_cnt = 0;
//						lcd_show_string(0, 0,24,buffer_x);
//						lcd_show_string(0, 32,24,buffer_y);
//						lcd_show_string(0, 64,24,buffer_z);
//					}
//				}
//			}

//		#endif

static void Display_Window_DCBrushMotor(void) 
{//直流电机操作界面
	uint8_t i;
	
	if(lcd_linebuff != lcd_line)
	{
		lcd_linebuff = lcd_line;
		for(i=0; i<MOTOR_NUM_MAX; i++)
		{
			if(lcd_linebuff == i)
			{
				lcd_set_color(WHITE, BLUE);
				lcd_show_string(2,2+20*i, 12, (char*)display[i]);
			}
			else
			{
				lcd_set_color(BLUE, WHITE);
				lcd_show_string(2,2+20*i, 12, (char*)display[i]);
			}
			lcd_set_color(BLUE, WHITE);
			lcd_show_string(2+64*(i%2), 2+20*(i/2+4),12,(char*)display[4+i]);
		}
	}
	/*******************直流电机数值***********************/
	for(i=0; i<MOTOR_NUM_MAX; i++)
	{
		if(SpeedValuebuff[i] != SpeedValue[i])
		{
			SpeedValuebuff[i] = SpeedValue[i];
			lcd_show_snum(86,2+(20*i),12,SpeedValue[i],4,0);
		}
		if(ReadSpeedValuebuff[i] != ReadSpeedValue[i])
		{
			ReadSpeedValuebuff[i] = ReadSpeedValue[i];
			lcd_show_snum(40+64*(i%2), 2+20*(i/2+4),12,ReadSpeedValue[i],4,0);
		}
	}
}
static void Display_Window_EncoderCalibrating(void)//编码轮校准界面
{
	if(lcd_linebuff != lcd_line)
	{
		lcd_linebuff = lcd_line;
	}
	if(StartTemperature != 0)
	{
		lcd_show_num(72,5+32, 12,StartTemperature,4,0);
		StartTemperature = 0;
	}
	if(CalibratResults == 1)
	{
		CalibratResults = 0;
		lcd_show_string(2+50,5+48, 12, "Done!");//校准完成！
		buzzerSound(H4);
		My_mDelay(120);
		buzzerSound(H5);
		My_mDelay(100);
		buzzerSound(H6);
		My_mDelay(120);
		buzzerSound(0);
	}
}
/*******循迹传感器读取*******/
static void Display_Window_TrackingSensorBool(void)
{
	uint8_t i,j;
	
	if(lcd_linebuff != lcd_line)
	{
		lcd_linebuff = lcd_line;
		for(i=0; i<TRACKINGSENSOR_NUM_MAX; i++)
		{
			if(lcd_linebuff == i)
			{
				lcd_set_color(WHITE, BLUE);
				lcd_show_string(2,2+30*i, 12, (char*)display[8+i]);
			}
			else
			{
				lcd_set_color(BLUE, WHITE);
				lcd_show_string(2,2+30*i, 12, (char*)display[8+i]);
			}
		}
	}
	/*******************循迹传感器数值***********************/
	for(i=0; i<TRACKINGSENSOR_NUM_MAX; i++)
	{
		if((TrackingValue[i] != TrackingValuebuff[i])
		 ||(TrackingState[i] == TRACKING_MODE_CALC))
		{
			if(TrackingState[i] == TRACKING_MODE_CALC)
			{
				lcd_set_color(BLUE, RED);
			}
			else
			{
				lcd_set_color(BLUE, GREEN);
			}
			TrackingValuebuff[i] = TrackingValue[i];
			for(j=0; j<8; j++)
			{
				if(((TrackingValuebuff[i]>>j)&0x01) == 0x01)
				{
					LCD_ShowString_BufMode(2+16*j,16+(30*i), "●",16,0);
				}
				else
				{
					LCD_ShowString_BufMode(2+16*j,16+(30*i), "○",16,0);
				}
			}
			lcd_set_color(BLUE, WHITE);
		}
	}
}
/*******超声波传感器读取*******/
static void Display_Window_UltrasonicSensor(void)
{
	uint8_t i;
	uint16_t Distance;
	
	if(lcd_linebuff != lcd_line)
	{
		lcd_linebuff = lcd_line;
	}
	if(Tim_CheckTimeOut(Time_buff, Time_Cnt, 100))
	{
		for(i=0; i<4; i++)
		{
//			Distance = SLAVE_UltrasonicRanging_DistanceRead(&UltrasonicRanging_S,i);
//			if(UltrasonicRangingValuebuff[i] != Distance)
//			{
//				UltrasonicRangingValuebuff[i] = Distance;
				lcd_show_num(36+(((i%2)*66)==66?66:2), 2+8*(8+i/2*3),12,UltrasonicRangingValuebuff[i],4,0);
//			}
		}
	}
}
static void Display_Window_JY62_MPU6050Sensor(void)
{
	uint8_t i;
	float angle;
	uint16_t temp;
	
	if(lcd_linebuff != lcd_line)
	{
		lcd_linebuff = lcd_line;
	}
	for(i=0; i<3; i++)
	{
		if(stcAngleBuff[i] != GyroData_Struct.stcAngle[i])
		{
			stcAngleBuff[i] = GyroData_Struct.stcAngle[i];
			angle = stcAngleBuff[i]*180/32768.0f;
			temp = angle*10;
			lcd_show_num(2+40, 16*i+21,12,temp/10,3,0);
			lcd_show_num(2+63, 16*i+21,12,temp%10,1,0);
		}
	}
}
static void Display_Window_BUS_ServoSensor(void)
{
	uint8_t i;
	
	if(lcd_linebuff != lcd_line)
	{
		lcd_linebuff = lcd_line;
	}
	for(i=0; i<6; i++)
	{
		lcd_show_num(2+90, (16*i) + 2, 12, BUS_Position[i],4,0);
	}
}
void Display_ChangeData(_e_DisplayOperate mode)
{
	void *datap;
	uint16_t DataMAX;
	int16_t  DataMin;
	uint16_t DataStep;
	
	//区别是哪一个显示界面
	if(LCD_Display_Hand == Display_Window_DCBrushMotor)
	{
		if(lcd_line >= MOTOR_NUM_MAX)
		{
			lcd_line = MOTOR_NUM_MAX-1;
		}
		datap = &SpeedValue[lcd_line];
		DataMAX = 32000;
		DataMin = -32000;
		DataStep = 20;
	}
	if(DisplayAdd == mode)
	{
		if((LCD_Display_Hand == Display_Window_DCBrushMotor))
		{
			if(*((int16_t*)datap) <= (DataMAX-DataStep))
			{
				*((int16_t*)datap) += DataStep;
			}
			else if(*((int16_t*)datap) < DataMAX)
			{
				*((int16_t*)datap) = DataMAX;
			}
		}
	}
	else if(DisplayMinus == mode)
	{
		if((LCD_Display_Hand == Display_Window_DCBrushMotor))
		{
			if(*((int16_t*)datap) >= (DataMin+DataStep))
			{
				*((int16_t*)datap) -= DataStep;
			}
			else if(*((int16_t*)datap) > DataMin)
			{
				*((int16_t*)datap) = DataMin;
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
void Display_Data_Init(void)
{
	lcd_page = 0;
	lcd_line = 0;
	lcd_linebuff = 0xff;
	window_flg = 0;
	LCD_Display_Hand = DisplayWindow[0];
}
void Display_Data(void)  //显示数据
{
	if(LCD_Display_Hand != 0)
	{
		LCD_Display_Hand();
	}
}
void Display_TurnPage(_e_DisplayOperate mode) //翻页操作
{
	uint8_t err;//标记为页是否发生改变

	if((1 == window_flg)&&(lcd_linebuff == lcd_line))
	{
		err = 1;	
		if((DisplayAdd == mode)&&(lcd_page < (WINDOW_NUM-1))) 
		{//最大为（WINDOW_NUM-1）页
			err = 0;
			lcd_page++;
		}
		else if((DisplayMinus == mode)&&(lcd_page > 1))
		{//最小为第一页，0页为清屏页，别的才为数据页
			err = 0;
			lcd_page--;
		}
		if(0 == err)
		{
			window_flg = 0;
			LCD_Display_Hand = DisplayWindow[0];//开始清屏准备换页
		}
	}
}
void Display_NewLine(_e_DisplayOperate mode) //换行操作
{
	uint8_t nummax;
	
	//区别是哪一个显示界面
	if(LCD_Display_Hand == Display_Window_TrackingSensorBool)
	{
		nummax = TRACKINGSENSOR_NUM_MAX-1;//4个，编号0~3
	}
	else if(LCD_Display_Hand == Display_Window_DCBrushMotor)
	{
		nummax = MOTOR_NUM_MAX-1;//4个，编号0~3
	}
	if( ( LCD_Display_Hand == Display_Window_DCBrushMotor ) || \
		( LCD_Display_Hand == Display_Window_TrackingSensorBool ) )
	{
		if((DisplayAdd == mode)&&(lcd_line < nummax))
		{
			lcd_line++;
		}
		else if((DisplayMinus == mode)&&(lcd_line > 0))
		{
			lcd_line--;
		}
	}
}
void Display_ENTER(void)
{
	if(LCD_Display_Hand == Display_Window_TrackingSensorBool)
	{
		switch(lcd_line)
		{
			case 0:
				SLAVE_Tracking_CheckModeEnable(&Tracking_Device1);
				break;
			case 1:
				SLAVE_Tracking_CheckModeEnable(&Tracking_Device2);
				break;
			case 2:
				SLAVE_Tracking_CheckModeEnable(&Tracking_Device3);
				break;
			case 3:
				SLAVE_Tracking_CheckModeEnable(&Tracking_Device4);
				break;
		}
	}
}
/** @brief  按键事件
  * @param  None
  * @retval None
  */
static void XferExternalKey1_Handler(void* btn)
{
	switch(((KEY_T*)btn)->event_flg)
	{
		case SIGNAL_SINGLE_CLICK:
			if( LCD_Display_Hand == Display_Window_StartPage )
			{
				if( RecognitionModule_t.RecognitionModuleSte == RM_leisure )
				{
					RecognitionModule_Start( &RecognitionModule_t );
				}
				else
				{
					RecognitionModule_Stop( &RecognitionModule_t );
				}
			}   
			break;
		case SIGNAL_DOUBLE_CLICK:
			if(LCD_Display_Hand == Display_Window_StartPage)
				Display_TurnPage(DisplayAdd);
			break;
		case SIGNAL_PRESS_DOWN:
			if( ( LCD_Display_Hand != Display_Window_StartPage ) && ( LCD_Display_Hand != Display_Window_Clear ) )
				Display_TurnPage(DisplayAdd);
			break;
	}
}
static void XferExternalKey2_Handler(void* btn)
{
    switch(((KEY_T*)btn)->event_flg)
    {
			case SIGNAL_PRESS_DOWN:
				Display_TurnPage(DisplayMinus);
				break;
    }
}
static void XferExternalKey3_Handler(void* btn)
{
    switch(((KEY_T*)btn)->event_flg)
    {
			case SIGNAL_SINGLE_CLICK: 
				Display_NewLine(DisplayAdd);			
				break; 
			case SIGNAL_DOUBLE_CLICK: 
				Display_ChangeData(DisplayAdd);			
				break; 
    }
}
static void XferExternalKey4_Handler(void* btn)
{
    switch(((KEY_T*)btn)->event_flg)
    {
			case SIGNAL_SINGLE_CLICK: 
				Display_NewLine(DisplayMinus);			
				break; 
			case SIGNAL_DOUBLE_CLICK: 
				Display_ChangeData(DisplayMinus);			
				break; 
    }
}
static void XferExternalKey5_Handler(void* btn)
{
    switch(((KEY_T*)btn)->event_flg)
    {
			case SIGNAL_PRESS_DOWN: 
				Display_ENTER();
				break;
    }
}

/***********************************************************************************************

                                       应用任务函数

************************************************************************************************/
void gui_task(void *pvParameters)
{
	int i;
		static uint8_t _lcd_show_timeout = 0;
	Display_Data_Init();
	while(1)
	{
		
		for(i = 0; i<HC_SR04_NUM; i++)
		{
			HCSR04_DistanceCalc(&sHCSR04Data[i],&UltrasonicRangingValuebuff[i]);
		}
		DCMotor_Ctrl_Scan();
		TrackingSensor_Ctrl_Scan();
		BUSPosition_Ctrl_Scan();
		mybtn_ticks();
		My_mDelay(KEYTICKS_INTERVAL-1);
		Display_Tick(KEYTICKS_INTERVAL);
		Display_Data();
	}
}
/***********************************************************************************************

                                       应用主函数

************************************************************************************************/

/* ------ manual test: display QR task code on LCD ------ */


int Task_GUI_create(void)
{
	/**********************按键初始化**************************/  
	mybtn_init( &key1_struct, KEY_1, 0, 300, 1200 );
	mybtn_init( &key2_struct, KEY_2, 0, 300, 1200 );
	mybtn_init( &key3_struct, KEY_3, 0, 300, 1200 );
	mybtn_init( &key4_struct, KEY_4, 0, 300, 1200 );
	mybtn_init( &key5_struct, KEY_5, 0, 300, 1200 );
	
	mybtn_attach( &key1_struct, PRESS_DOWN, XferExternalKey1_Handler);
	mybtn_attach( &key1_struct, SINGLE_CLICK, XferExternalKey1_Handler);
	mybtn_attach( &key1_struct, DOUBLE_CLICK, XferExternalKey1_Handler);
	mybtn_attach( &key2_struct, PRESS_DOWN, XferExternalKey2_Handler);
	mybtn_attach( &key3_struct, PRESS_DOWN, XferExternalKey3_Handler);
	mybtn_attach( &key3_struct, SINGLE_CLICK, XferExternalKey3_Handler);
	mybtn_attach( &key3_struct, DOUBLE_CLICK, XferExternalKey3_Handler);
	mybtn_attach( &key4_struct, PRESS_DOWN, XferExternalKey4_Handler);
	mybtn_attach( &key4_struct, SINGLE_CLICK, XferExternalKey4_Handler);
	mybtn_attach( &key4_struct, DOUBLE_CLICK, XferExternalKey4_Handler);
	mybtn_attach( &key5_struct, PRESS_DOWN, XferExternalKey5_Handler);
		
	mybtn_start( &key1_struct );
	mybtn_start( &key2_struct );
	mybtn_start( &key3_struct );
	mybtn_start( &key4_struct );
	mybtn_start( &key5_struct );
	
	thread_gui = rt_thread_create("gui",           /* 线程名字 */
								 gui_task,       /* 线程入口函数 */
								 RT_NULL,        /* 线程入口函数参数 */
								 512,            /* 线程栈大小 */
								 RT_THREAD_PRIORITY_MAX/2,/* 线程的优先级 */
								 20);            /* 线程时间片 */
	if(thread_gui != RT_NULL)
	{
		rt_thread_startup(thread_gui);
		rt_kprintf("thread_gui startup!\n");
	}
	return 0;
}
INIT_APP_EXPORT(Task_GUI_create);

