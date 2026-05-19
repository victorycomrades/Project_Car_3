/* Includes ------------------------------------------------------------------*/
#include "thread_user.h"
/* 开发板数据文件 */
#include "data.h"
/* 其他任务头文件 */
#include "thread_rccu.h"
#include "thread_gui.h"
#include "thread_comm.h"
#include "SCServo.h"
#include "Servo.h"
/* Private macros ------------------------------------------------------------*/
int16_t wait = 0;
int16_t wait_locXY = 0;
uint16_t Color_wait = WHITE;
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* 定义线程控制块指针 */
rt_thread_t thread_PathWrite = RT_NULL;
/*                            */
int32_t PositionXmm_Old;
int32_t PositionYmm_Old;
int32_t PositionXmm_Diff;
int32_t PositionYmm_Diff;
__IO uint16_t run_cnt = 0;
/* Private functions ---------------------------------------------------------*/
static void Debug_Await(void)
{
	while( (KEY_5() == 0) || (lcd_page != 1))
		My_mDelay(50);
	while( (KEY_5() == 1) || (lcd_page != 1))
		My_mDelay(50);
}

static void Wait_Start(void)//等待初始化
{
	while(Wheel_isReady != 2)
		My_mDelay(50);
}
/*字符串拼接*/
void Stract(char strDestination[],char strSource[],int num)
{
	int i = 0,j = 0;
	while(strDestination[i]!='\0') i++;
	for(j = 0;j<num;j++)
			strDestination[i++] = strSource[j];
}
void Update_X(float New_X)
{
	char Update_x[8] = "ACTX";
	static union
	{
		float X;
		char data[4];
	}New_set;
	New_set.X = New_X;
	Stract(Update_x,New_set.data,4);
	Bsp_UARTMixed_TxTrigger(&muart4, Update_x, 8);
}
void Update_Y(float New_Y)
{
	char Update_y[8] = "ACTY";
	static union
	{
		float Y;
		char data[4];
	}New_set;
	New_set.Y = New_Y;
	Stract(Update_y,New_set.data,4);
	Bsp_UARTMixed_TxTrigger(&muart4, Update_y, 8);
}
void Update_J(float New_J)
{
	char Update_j[8] = "ACTJ";
	static union
	{
		float J;
		char data[4];
	}New_set;
	New_set.J = New_J;
	Stract(Update_j,New_set.data,4);
	Bsp_UARTMixed_TxTrigger(&muart4, Update_j, 8);
}
static void SetCoordinateXY(float SET_LOCATION_X, float SET_LOCATION_Y)//设置编码轮坐标
{
		float SET_LOCATION_J;
		SET_LOCATION_J = Read_Position_yaw();//读取车当前航向轴
		ChassisRELAX_set();//切换自由模式
		Update_X(SET_LOCATION_X);
		HAL_Delay(10);
		Update_Y(SET_LOCATION_Y);
		HAL_Delay(10);
		Update_J(SET_LOCATION_J);
		HAL_Delay(50);
		ChassisCoord_Set(SET_LOCATION_X,SET_LOCATION_Y,0);//设置坐标
		ChassisCoord_WaitStop();// 等待车身稳定
		HAL_Delay(50);
}
static void ResetCoordinateXYJ(void)//编码轮坐标清零
{
		ChassisRELAX_set();//切换自由模式
		Bsp_UARTMixed_TxTrigger(&muart4, "ACT0", 4);
		HAL_Delay(50);
		ChassisCoord_Set(0,0,0);//切换坐标模式
}

static void Identify_circle_Seq(void)//识别圆环颜色顺序，1为红，2为绿，3为蓝
{
	uint8_t i;
	bool_recognitionflag = 0;
	CircleSequenceRecognitionModule_Start(&PCModule_t);
	while( PCModule_t.RecognitionModuleSte != RM_succeed )//等待数据
	{
		My_mDelay(50);
	}
	for(i=0; i<3; i++)  
	{//识别到的字符串对应颜色
		switch(Recognition_Buffer[i])
		{
			case '1':
				CircleSequence[i] = RED;
				break;
			case '2':
				CircleSequence[i] = GREEN;
				break;
			case '3':
				CircleSequence[i] = BLUE;
				break;
		}
	}
	bool_recognitionflag = 0;
	bool_circlesequenceflag = 1;
	RecognitionModule_Stop(&PCModule_t);
}


/******************************************************************************/
/* 视觉模块串口朝前，182行校准的xy方向为ChassisSpeed_Set(-yout,-xout);        */
/* 视觉模块串口朝后，182行校准的xy方向为ChassisSpeed_Set(yout,xout);        */
/* 视觉模块串口朝左，182行校准的xy方向为ChassisSpeed_Set(xout,-yout);       */
/* 视觉模块串口朝右，182行校准的xy方向为ChassisSpeed_Set(-xout,yout);        */
/******************************************************************************/

static void IdentifyCircle_Get(float SET_LOCATION_X, float SET_LOCATION_Y)//识别靶心并调整位置
{
	double y_err[2] = {0,0};
	double x_err[2] = {0,0};
	double yout,xout;
	bool_recognitionflag = 0;
	CircleRecognitionModule_Start(&PCModule_t);
	while(1)
	{
		My_mDelay(10);
		wait++;
		wait_locXY++;
		if(PCModule_t.RecognitionModuleSte == RM_error)
		{
			My_mDelay(500);
			bool_recognitionflag = 0;
			CircleRecognitionModule_Start(&PCModule_t);
		}
		else if(PCModule_t.RecognitionModuleSte == RM_succeed)
		{// 积分偏差计算，算出移动速度
			x_err[1] = x_err[0];
			x_err[0] = SET_LOCATION_X - cartesian_loc.x;
			xout = ( 1.5 * x_err[0] ) + ( 1.5f * ( x_err[0] - x_err[1] ) );
			if( xout > 50 ) xout = 50;
		    else if( xout < -50 )xout = -50;
			
			y_err[1] = y_err[0];
			y_err[0] = SET_LOCATION_Y - cartesian_loc.y;
			yout = ( 1.5 * y_err[0] ) + ( 1.5f * ( y_err[0] - y_err[1] ) );
			if( yout > 50 ) yout = 50;
		    else if( yout < -50 )yout = -50;
			
			if( ( ABS(y_err[0]) <= 2 ) && ( ABS(x_err[0]) <= 2 ) )// 当xy偏差值小于等于1时，停止调整
			{
				ChassisSpeed_Set(0,0);// 停止移动
				wait = 0;
				break;
			}
			else
			{
				ChassisSpeed_Set(-yout,-xout);
			}
			PCModule_t.RecognitionModuleSte = RM_Identify;
		}
//		else if(wait_locXY > 50)//超时未接收到坐标，车辆暂停移动
//		{
//			ChassisSpeed_Set(0,0);// 停止移动
//		}
		else if(wait == 1500)//整体超时过长自动跳出循环结束任务
		{
			wait = 0;
			ChassisSpeed_Set(0,0);// 停止移动
			break;
		}

	}
//	Rest_action();//归位
//	Rest_action_S2();
	RecognitionModule_Stop(&PCModule_t);
}
/**
 * @brief  二维码识别时动作
 * @param  空
 * @return 空
 * @explain 该函数打开二维码识别后，在等待bool_recognitionflag==1的过程中，使小车左右移动
			以此增加识别概率，用户可以通过更改Time_Wait的判断值来调整左移右移的时间
相关函数：	XferExternalUart3Rx_Handler_New(接收二维码模块传来的数据并将bool_recognitionflag置1)
 */
void Identify_QRCode_New(void) //在未识别到二维码时，进行左右摆动
{
	RecognitionModule_Start(&RecognitionModule_t);//打开二维码识别
	static uint32_t Time_Wait=0;	//计时量
	while(bool_recognitionflag==0)  //XferExternalUart3Rx_Handler_New会将标志位置1，代表接收到数据
	{
		ChassisSpeed_Set(30,0);
		My_mDelay(2000);
		ChassisSpeed_Set(-30,0);
		My_mDelay(2000);
		ChassisSpeed_Set(0,0);
		Time_Wait++;
		if(Time_Wait == 2)
		{
			break;
		}
//		if(Time_Wait<1000)	ChassisSpeed_Set(-80,0);  //可更改Time_Wait的判断值，确保不会跑太远，当前值偏大
//		else 				ChassisSpeed_Set(80,0);
		My_mDelay(20);
	}
	
	bool_colorsequenceflag = 1;//屏幕显示标志位
}


static void Identify_QRCode(void)
{//识别二维码
	uint8_t j,i;
	int a = 0;
	int b = 0;
	
#if TESTDEBUG == 1
	Debug_Await();
#endif
	bool_recognitionflag = 0;
#if 0     //1模块  0摄像头
	RecognitionModule_Start(&RecognitionModule_t);
	while( RecognitionModule_t.RecognitionModuleSte != RM_succeed )
	{
		a++;
		My_mDelay(50);
		if(a > 60) 
		{
			ChassisSpeed_Set(0,30);
			My_mDelay(2000);
			ChassisSpeed_Set(0,-30);
			My_mDelay(2000);
			ChassisSpeed_Set(0,0);
		}
	}
#else
	RecognitionModule_Start(&PCModule_t);
//	while( PCModule_t.RecognitionModuleSte != RM_succeed )
	while(bool_recognitionflag==0)
	{
		a++;
		My_mDelay(50);
		if(a > 60) 
		{
			ChassisSpeed_Set(0,30);
			My_mDelay(2000);
			ChassisSpeed_Set(0,-30);
			My_mDelay(2000);
			ChassisSpeed_Set(0,0);
			b++;
			if(b == 2)
			{
				Identify_QRCode_New();
			}
		}
		
		
	}
#endif
	RecognitionModule_Stop(&PCModule_t);
	for(j=0; j<2; j++)
	{
		for(i=0; i<3; i++)  
		{//识别到的字符串对应颜色
			switch(Recognition_Buffer[(4*j)+i])
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
			}
		}
	}
	bool_recognitionflag = 0;
	bool_colorsequenceflag = 1;
}
void Pause_ColorRecognition()//暂停颜色识别
{
		RecognitionModule_Stop(&PCModule_t);//停止视觉主机目前所有执行任务
		bool_recognitionflag = 0;
		CircleRecognitionModule_Start(&PCModule_t);
}
void Renew_ColorRecognition()//恢复颜色识别
{
		RecognitionModule_Stop(&PCModule_t);//停止视觉主机目前所有执行任务
		bool_recognitionflag = 0;
		ColorRecognitionModule_Start(&PCModule_t);
}

static void IdentifyColor_Get(uint8_t mode)//识别颜色并抓取；第一趟mode=0，第二趟mode=1
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//车身停用定位系统矫正
	#endif
	uint8_t i = 0;
	if(mode > 1)
		mode = 1;
	bool_recognitionflag = 0;
	while(1)
	{
		My_mDelay(10);
			switch(ColorSequence[mode][i])
			{
				case RED:
					Get_1();
					break;
				case GREEN:
					Get_2();
					break;
				case BLUE:
					Get_3();
					break;
				default:
					break;
			}
			IdentifyColor_action();//识别原料区动作
			Renew_ColorRecognition();//恢复颜色识别	
			while(1)
			{
				if(CurrentMaterialColor == ColorSequence[mode][i])
				{//颜色识别成功判断颜色
					//进行抓取
					Pause_ColorRecognition();//暂停颜色识别
	//				if (mode ==0)
	//				{
	//					Put_A();
	//				}
	//				else
	//				{
	//					Put_B();
	//				}
					Put_A();
					i++;
					break;
				}
				My_mDelay(30);
			}
//			PCModule_t.RecognitionModuleSte = RM_Identify;
//			bool_recognitionflag = 0;
//			My_mDelay(500);
		if(i==3)
			break;
	}
	Rest_action_S2();
	RecognitionModule_Stop(&PCModule_t);
}
static void Circle_Get(float SET_LOCATION_X, float SET_LOCATION_Y)//识别靶心并调整位置
{
	double y_err[2] = {0,0};
	double x_err[2] = {0,0};
	double yout,xout;
	bool_recognitionflag = 0;
	CircleRecognitionModule_Start(&PCModule_t);
	while(1)
	{
		My_mDelay(10);
		wait++;
		wait_locXY++;
		if(PCModule_t.RecognitionModuleSte == RM_error)
		{
			My_mDelay(500);
			bool_recognitionflag = 0;
			CircleRecognitionModule_Start(&PCModule_t);
		}
		else if(PCModule_t.RecognitionModuleSte == RM_succeed)
		{// 积分偏差计算，算出移动速度
			x_err[1] = x_err[0];
			x_err[0] = SET_LOCATION_X - cartesian_loc.x;
			xout = ( 1.5 * x_err[0] ) + ( 1.5f * ( x_err[0] - x_err[1] ) );
			if( xout > 50 ) xout = 50;
		    else if( xout < -50 )xout = -50;
			
			y_err[1] = y_err[0];
			y_err[0] = SET_LOCATION_Y - cartesian_loc.y;
			yout = ( 1.5 * y_err[0] ) + ( 1.5f * ( y_err[0] - y_err[1] ) );
			if( yout > 50 ) yout = 50;
		    else if( yout < -50 )yout = -50;
			
			if( ( ABS(y_err[0]) <= 10 ) && ( ABS(x_err[0]) <= 10 ) )// 当xy偏差值小于等于1时，停止调整
			{
				ChassisSpeed_Set(0,0);// 停止移动
				wait = 0;
				break;
			}
			else
			{
				ChassisSpeed_Set(-yout,-xout);
			}
			PCModule_t.RecognitionModuleSte = RM_Identify;
		}
//		else if(wait_locXY > 50)//超时未接收到坐标，车辆暂停移动
//		{
//			ChassisSpeed_Set(0,0);// 停止移动
//		}
		else if(wait == 1500)//整体超时过长自动跳出循环结束任务
		{
			wait = 0;
			ChassisSpeed_Set(0,0);// 停止移动
			break;
		}

	}
	RecognitionModule_Stop(&PCModule_t);
}
static void Color_Get(uint8_t mode)//识别颜色并抓取；第一趟mode=0，第二趟mode=1
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//车身停用定位系统矫正
	#endif
	uint8_t i = 0;
	if(mode > 1)
		mode = 1;
	bool_recognitionflag = 0;
	ColorRecognitionModule_Start(&PCModule_t);
	while(1)
	{
		My_mDelay(10);
		if(PCModule_t.RecognitionModuleSte == RM_error)
		{
			My_mDelay(300);
			bool_recognitionflag = 0;
			ColorRecognitionModule_Start(&PCModule_t);
		}
		else if(PCModule_t.RecognitionModuleSte == RM_succeed)
		{

			while(Color_wait == CurrentMaterialColor)
			{
				My_mDelay(10);	
			}
			Color_wait=CurrentMaterialColor;
			while(Color_wait == CurrentMaterialColor)
			{
				My_mDelay(10);	
			}
			Circle_Get(100,150);
			break;

		}
		
	}
}

void RoughingArea_PutGet(uint8_t ColorSequenceNum)//粗加工区放置物料（圆环颜色固定红绿蓝情况）
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//车身停用定位系统矫正
	#endif
	uint8_t i;
		for(i=0; i<3; i++) //提取要取颜色索引号
		{
			if(ColorSequence[ColorSequenceNum][i] == RED){
			Get_1();
			Put_Red();
			}
			else if(ColorSequence[ColorSequenceNum][i] == GREEN){
			Get_2();
			Put_Green();
			}
			else if(ColorSequence[ColorSequenceNum][i] == BLUE){
			Get_3();
			Put_Blue();
			}
		}
		//Debug_Await();
		for(i=0; i<3; i++) //提取要取颜色索引号
		{
			if(ColorSequence[ColorSequenceNum][i] == RED){
			Get_Red();
			Put_1();
			}
			else if(ColorSequence[ColorSequenceNum][i] == GREEN){
			Get_Green();
			Put_2();
			}
			else if(ColorSequence[ColorSequenceNum][i] == BLUE){
			Get_Blue();
			Put_3();
			}
		}
	Rest_action_S2();
}

void FashioningArea_Put(uint8_t ColorSequenceNum)//精加工区放置物料（圆环颜色固定红绿蓝情况）
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//车身停用定位系统矫正
	#endif
	uint8_t i;
	if(ColorSequenceNum == 0){
		for(i=0; i<3; i++) //放下层物料
		{
			if(ColorSequence[ColorSequenceNum][i] == RED){
			Get_1();
			Put_Red();
			}
			else if(ColorSequence[ColorSequenceNum][i] == GREEN){
			Get_2();
			Put_Green();
			}
			else if(ColorSequence[ColorSequenceNum][i] == BLUE){
			Get_3();
			Put_Blue();
			}
		}
	}
	if(ColorSequenceNum == 1){
		for(i=0; i<3; i++) //码垛物料
		{
			if(ColorSequence[ColorSequenceNum][i] == RED){
			Get_1();
			Put_Red_Stacking();
			}
			else if(ColorSequence[ColorSequenceNum][i] == GREEN){
			Get_2();
			Put_Green_Stacking();
			}
			else if(ColorSequence[ColorSequenceNum][i] == BLUE){
			Get_3();
			Put_Blue_Stacking();
			}
		}
	}
	Rest_action_S2();
}


void RoughingArea_PutGet_R(uint8_t ColorSequenceNum)//粗加工区放置物料（圆环颜色随机情况）
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//车身停用定位系统矫正
	#endif
	uint8_t i;
		for (i = 0; i < 3; i++) 
		{
			if(ColorSequence[ColorSequenceNum][i] == RED) {
					Get_1();
					if (CircleSequence[0] == RED) {
							Put_Blue(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == RED) {
							Put_Green(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == RED) {
							Put_Red(); // 放从左往右数第三个圈
					}
			}
			else if (ColorSequence[ColorSequenceNum][i] == GREEN) {
					Get_2();
					if (CircleSequence[0] == GREEN) {
							Put_Blue(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == GREEN) {
							Put_Green(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == GREEN) {
							Put_Red(); // 放从左往右数第三个圈
					}
			}
			else if (ColorSequence[ColorSequenceNum][i] == BLUE) {
					Get_3();
					if (CircleSequence[0] == BLUE) {
							Put_Blue(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == BLUE) {
							Put_Green(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == BLUE) {
							Put_Red(); // 放从左往右数第三个圈
					}
			}
		}
		//Debug_Await();
		for(i=0; i<3; i++) //提取要取颜色索引号
		{
			if(ColorSequence[ColorSequenceNum][i] == RED){
				if (CircleSequence[0] == RED) {
						Get_Blue(); // 拿从左往右数第一个圈
				}
				else if (CircleSequence[1] == RED) {
						Get_Green(); // 拿从左往右数第二个圈
				}
				else if (CircleSequence[2] == RED) {
						Get_Red(); // 拿从左往右数第三个圈
				}
				Put_1();
			}
			else if(ColorSequence[ColorSequenceNum][i] == GREEN){
				if (CircleSequence[0] == GREEN) {
						Get_Blue(); // 拿从左往右数第一个圈
				}
				else if (CircleSequence[1] == GREEN) {
						Get_Green(); // 拿从左往右数第二个圈
				}
				else if (CircleSequence[2] == GREEN) {
						Get_Red(); // 拿从左往右数第三个圈
				}
				Put_2();
			}
			else if(ColorSequence[ColorSequenceNum][i] == BLUE){
				if (CircleSequence[0] == BLUE) {
						Get_Blue(); // 拿从左往右数第一个圈
				}
				else if (CircleSequence[1] == BLUE) {
						Get_Green(); // 拿从左往右数第二个圈
				}
				else if (CircleSequence[2] == BLUE) {
						Get_Red(); // 拿从左往右数第三个圈
				}
				Put_3();
			}
		}
}
void FashioningArea_Put_R(uint8_t ColorSequenceNum)//精加工区放置物料（圆环颜色随机情况）
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//车身停用定位系统矫正
	#endif
	uint8_t i;
	if(ColorSequenceNum == 0){
		for (i = 0; i < 3; i++)//放下层物料
		{
			if(ColorSequence[ColorSequenceNum][i] == RED) {
					
					if (CircleSequence[0] == RED) {
							Get_Blue(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == RED) {
							Get_Green(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == RED) {
							Get_Red(); // 放从左往右数第三个圈
					}
					Put_1();
			}
			else if (ColorSequence[ColorSequenceNum][i] == GREEN) {
					if (CircleSequence[0] == GREEN) {
							Get_Blue(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == GREEN) {
							Get_Green(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == GREEN) {
							Get_Red(); // 放从左往右数第三个圈
					}
					Put_2();
			}
			else if (ColorSequence[ColorSequenceNum][i] == BLUE) {
					if (CircleSequence[0] == BLUE) {
							Get_Blue(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == BLUE) {
							Get_Green(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == BLUE) {
							Get_Red(); // 放从左往右数第三个圈
					}
					Put_3();
			}
		}
	}

	if(ColorSequenceNum == 1){
		for (i = 0; i < 3; i++)//码垛物料
		{
			if(ColorSequence[ColorSequenceNum][i] == RED) {
					
					if (CircleSequence[0] == RED) {
							Get_Blue(); // 放从左往右数第一个圈
//							Get_Blue_Stacking(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == RED) {
							Get_Green(); // 放从左往右数第二个圈
//							Get_Green_Stacking(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == RED) {
							Get_Red(); // 放从左往右数第三个圈
//							Get_Red_Stacking(); // 放从左往右数第三个圈
					}
					Put_1();
			}
			else if (ColorSequence[ColorSequenceNum][i] == GREEN) {

					if (CircleSequence[0] == GREEN) {
							Get_Blue(); // 放从左往右数第一个圈
//							Get_Blue_Stacking(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == GREEN) {
							Get_Green(); // 放从左往右数第二个圈
//							Get_Green_Stacking(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == GREEN) {
							Get_Red(); // 放从左往右数第三个圈
//							Get_Red_Stacking(); // 放从左往右数第三个圈
					}
					Put_2();
			}
			else if (ColorSequence[ColorSequenceNum][i] == BLUE) {

					if (CircleSequence[0] == BLUE) {
							Get_Blue(); // 放从左往右数第一个圈
//							Get_Blue_Stacking(); // 放从左往右数第一个圈
					}
					else if (CircleSequence[1] == BLUE) {
							Get_Green(); // 放从左往右数第二个圈
//							Get_Green_Stacking(); // 放从左往右数第二个圈
					}
					else if (CircleSequence[2] == BLUE) {
							Get_Red(); // 放从左往右数第三个圈
//							Get_Red_Stacking(); // 放从左往右数第三个圈
					}
					Put_3();
			}
		}
}
}
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void PathWrite_task(void *pvParameters)
{//路劲规划任务
	RecognitionModule_Stop(&PCModule_t);
	Rest_action_S2();//舵机初始抬起
	ChassisModle_Set(2);
	while(1)
	{
	#if 1
		Debug_Await();// 等待按键被按下
		Wait_Start();// 等待编码轮初始化完成	
		//================出发扫码区================
		ChassisCoord_Set(0,-190, 0); 
		ChassisCoord_WaitStop();
		ChassisCoord_Set(-750,-190, 0);
		QR_action();//二维码识别动作
		ChassisCoord_WaitStop();
		//===============识别二维码===============
		Identify_QRCode();
		//==============前进到原料区==============
		//===============走中间===============
		ChassisCoord_Set(-1080, -180, 180);
		ChassisCoord_WaitStop();
		//=============移动到粗加工区=============→→→→→→→→→→→→→→→→→→→→→→→→→→→粗
		ChassisCoord_Set(-1080, -1905, 0);
		IdentifyCircle_action();
		ChassisCoord_WaitStop();
		
		IdentifyCircle_Get(100,150);//视觉识别圆心
		Get_color();    //同时能够看色环或物料顺序的机械臂动作角度
		Identify_circle_Seq();//识别圆环颜色顺序
		FashioningArea_Put_R(0); //色环随机粗加工动作
		SetCoordinateXY(0,0);//重置定位系统的坐标系，消除此前误差
		Rest_action_S2();
		
		ChassisCoord_Set(-850, 0, -90);
		ChassisCoord_WaitStop();
		//=============移动到精加工区=============→→→→→→→→→→→→→→→→→→→→→→→→→→→精
		ChassisCoord_Set(-848, 828, 0);
		ChassisCoord_WaitStop();
		IdentifyCircle_action();
		
		
		IdentifyCircle_Get(100,150);//视觉识别圆心
		Rest_action_S2();//复位动作
		RoughingArea_PutGet(0);
		SetCoordinateXY(0,0);//重置定位系统的坐标系，消除此前误差
		
		ChassisCoord_Set(0, 885, -90);
		ChassisCoord_WaitStop();
		ChassisCoord_Set(460, 885,0);
		ChassisCoord_WaitStop();
		
		IdentifyColor_action();
		Color_Get(0);
		
		IdentifyColor_Get(0);//将物料放到转盘
		
		SetCoordinateXY(-1530,-180);//重置定位系统转盘位置
		
		//===============走中间===============
		ChassisCoord_Set(-1090, -180, 180);
		ChassisCoord_WaitStop();	
		
		//=============移动到粗加工区=============→→→→→→→→→→→→→→→→→→→→→→→→→→→粗
		ChassisCoord_Set(-1090, -1905, 0);
		IdentifyCircle_action();
		ChassisCoord_WaitStop();
		
		IdentifyCircle_Get(100,150);//视觉识别圆心
		Get_color();    //同时能够看色环或物料顺序的机械臂动作角度
		Identify_circle_Seq();//识别圆环颜色顺序
		FashioningArea_Put_R(1); //色环随机粗加工动作
		SetCoordinateXY(0,0);//重置定位系统的坐标系，消除此前误差
		Rest_action_S2();
		
		ChassisCoord_Set(-850, 0, -90);
		ChassisCoord_WaitStop();
		//=============移动到精加工区=============→→→→→→→→→→→→→→→→→→→→→→→→→→→精
		ChassisCoord_Set(-850, 828, 0);
		IdentifyCircle_action();
		ChassisCoord_WaitStop();
		
		IdentifyCircle_Get(100,150);//视觉识别圆心
		Rest_action_S2();//复位动作
		RoughingArea_PutGet(1);
		SetCoordinateXY(0,0);//重置定位系统的坐标系，消除此前误差
		
		ChassisCoord_Set(0, 885, -90);
		ChassisCoord_WaitStop();
		ChassisCoord_Set(420, 897,0);
		ChassisCoord_WaitStop();
		
		IdentifyColor_action();
		Color_Get(0);
		
		IdentifyColor_Get(1);//将物料放到转盘
		SetCoordinateXY(-1530,-180);//重置定位系统转盘位置
//===============回出发区===============
		ChassisCoord_Set(-720,-190, 0);
		ChassisCoord_WaitStop();
		ChassisCoord_Set(0,-190, 0); 
		ChassisCoord_WaitStop();
		ChassisCoord_Set(0,0, 0); 
		ChassisCoord_WaitStop();
		
		


		
	#else
		Debug_Await();// 等待按键被按下
		Wait_Start();// 等待编码轮初始化完成
		
		IdentifyColor_Get(0);//将物料放到转盘
		
		
		
	
	
		#endif
		//*/
		Debug_Await();// 等待下一次任务
	}
}
int Task_User_create(void)
{
	thread_PathWrite = rt_thread_create( "PathWrite",             /* 线程名字 */
								         PathWrite_task,          /* 线程入口函数 */
								         RT_NULL,                 /* 线程入口函数参数 */
								         1024,          		  /* 线程栈大小 */
								         10,                      /* 线程的优先级 */
								         20);                     /* 线程时间片 */
	if(thread_PathWrite != RT_NULL)
	{
		rt_thread_startup(thread_PathWrite);
		rt_kprintf("thread_PathWrite startup!\n");
	}
	
	return 0;
}
