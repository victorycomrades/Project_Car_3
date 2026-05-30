/* Includes ------------------------------------------------------------------*/
#include "thread_user.h"
/* �����������ļ� */
#include "data.h"
/* ��������ͷ�ļ� */
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
/* �����߳̿��ƿ�ָ�� */
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

static void Wait_Start(void)//�ȴ���ʼ��
{
	while(Wheel_isReady != 2)
		My_mDelay(50);
}
/*�ַ���ƴ��*/
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
static void SetCoordinateXY(float SET_LOCATION_X, float SET_LOCATION_Y)//���ñ���������
{
		float SET_LOCATION_J;
		SET_LOCATION_J = Read_Position_yaw();//��ȡ����ǰ������
		ChassisRELAX_set();//�л�����ģʽ
		Update_X(SET_LOCATION_X);
		HAL_Delay(10);
		Update_Y(SET_LOCATION_Y);
		HAL_Delay(10);
		Update_J(SET_LOCATION_J);
		HAL_Delay(50);
		ChassisCoord_Set(SET_LOCATION_X,SET_LOCATION_Y,0);//��������
		ChassisCoord_WaitStop();// �ȴ������ȶ�
		HAL_Delay(50);
}
void ResetCoordinateXYJ(void)//��������������
{
		ChassisRELAX_set();//�л�����ģʽ
		Bsp_UARTMixed_TxTrigger(&muart4, "ACT0", 4);
		HAL_Delay(50);
		ChassisCoord_Set(0,0,0);//�л�����ģʽ
}

static void Identify_circle_Seq(void)//ʶ��Բ����ɫ˳��1Ϊ�죬2Ϊ�̣�3Ϊ��
{
	uint8_t i;
	bool_recognitionflag = 0;
	CircleSequenceRecognitionModule_Start(&PCModule_t);
	while( PCModule_t.RecognitionModuleSte != RM_succeed )//�ȴ�����
	{
		My_mDelay(50);
	}
	for(i=0; i<3; i++)  
	{//ʶ�𵽵��ַ�����Ӧ��ɫ
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
/* �Ӿ�ģ�鴮�ڳ�ǰ��182��У׼��xy����ΪChassisSpeed_Set(-yout,-xout);        */
/* �Ӿ�ģ�鴮�ڳ���182��У׼��xy����ΪChassisSpeed_Set(yout,xout);        */
/* �Ӿ�ģ�鴮�ڳ���182��У׼��xy����ΪChassisSpeed_Set(xout,-yout);       */
/* �Ӿ�ģ�鴮�ڳ��ң�182��У׼��xy����ΪChassisSpeed_Set(-xout,yout);        */
/******************************************************************************/

static void IdentifyCircle_Get(float SET_LOCATION_X, float SET_LOCATION_Y)//ʶ����Ĳ�����λ��
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
		{// ����ƫ����㣬����ƶ��ٶ�
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
			
			if( ( ABS(y_err[0]) <= 2 ) && ( ABS(x_err[0]) <= 2 ) )// ��xyƫ��ֵС�ڵ���1ʱ��ֹͣ����
			{
				ChassisSpeed_Set(0,0);// ֹͣ�ƶ�
				wait = 0;
				break;
			}
			else
			{
				ChassisSpeed_Set(-yout,-xout);
			}
			PCModule_t.RecognitionModuleSte = RM_Identify;
		}
//		else if(wait_locXY > 50)//��ʱδ���յ����꣬������ͣ�ƶ�
//		{
//			ChassisSpeed_Set(0,0);// ֹͣ�ƶ�
//		}
		else if(wait == 1500)//���峬ʱ�����Զ�����ѭ����������
		{
			wait = 0;
			ChassisSpeed_Set(0,0);// ֹͣ�ƶ�
			break;
		}

	}
//	Rest_action();//��λ
//	Rest_action_S2();
	RecognitionModule_Stop(&PCModule_t);
}
/**
 * @brief  ��ά��ʶ��ʱ����
 * @param  ��
 * @return ��
 * @explain �ú����򿪶�ά��ʶ����ڵȴ�bool_recognitionflag==1�Ĺ����У�ʹС�������ƶ�
			�Դ�����ʶ����ʣ��û�����ͨ������Time_Wait���ж�ֵ�������������Ƶ�ʱ��
��غ�����	XferExternalUart3Rx_Handler_New(���ն�ά��ģ�鴫�������ݲ���bool_recognitionflag��1)
 */
void Identify_QRCode_New(void) //��δʶ�𵽶�ά��ʱ���������Ұڶ�
{
	RecognitionModule_Start(&RecognitionModule_t);//�򿪶�ά��ʶ��
	static uint32_t Time_Wait=0;	//��ʱ��
	while(bool_recognitionflag==0)  //XferExternalUart3Rx_Handler_New�Ὣ��־λ��1���������յ�����
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
//		if(Time_Wait<1000)	ChassisSpeed_Set(-80,0);  //�ɸ���Time_Wait���ж�ֵ��ȷ��������̫Զ����ǰֵƫ��
//		else 				ChassisSpeed_Set(80,0);
		My_mDelay(20);
	}
	
	bool_colorsequenceflag = 1;//��Ļ��ʾ��־λ
}


static void Identify_QRCode(void)
{//ʶ���ά��
	uint8_t j,i;
	int a = 0;
	int b = 0;
	
#if TESTDEBUG == 1
	Debug_Await();
#endif
	bool_recognitionflag = 0;
#if 0     //1ģ��  0����ͷ
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
		{//ʶ�𵽵��ַ�����Ӧ��ɫ
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
void Pause_ColorRecognition()//��ͣ��ɫʶ��
{
		RecognitionModule_Stop(&PCModule_t);//ֹͣ�Ӿ�����Ŀǰ����ִ������
		bool_recognitionflag = 0;
		CircleRecognitionModule_Start(&PCModule_t);
}
void Renew_ColorRecognition()//�ָ���ɫʶ��
{
		RecognitionModule_Stop(&PCModule_t);//ֹͣ�Ӿ�����Ŀǰ����ִ������
		bool_recognitionflag = 0;
		ColorRecognitionModule_Start(&PCModule_t);
}

static void IdentifyColor_Get(uint8_t mode)//ʶ����ɫ��ץȡ����һ��mode=0���ڶ���mode=1
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//����ͣ�ö�λϵͳ����
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
			IdentifyColor_action();//ʶ��ԭ��������
			Renew_ColorRecognition();//�ָ���ɫʶ��	
			while(1)
			{
				if(CurrentMaterialColor == ColorSequence[mode][i])
				{//��ɫʶ��ɹ��ж���ɫ
					//����ץȡ
					Pause_ColorRecognition();//��ͣ��ɫʶ��
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
static void Circle_Get(float SET_LOCATION_X, float SET_LOCATION_Y)//ʶ����Ĳ�����λ��
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
		{// ����ƫ����㣬����ƶ��ٶ�
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
			
			if( ( ABS(y_err[0]) <= 10 ) && ( ABS(x_err[0]) <= 10 ) )// ��xyƫ��ֵС�ڵ���1ʱ��ֹͣ����
			{
				ChassisSpeed_Set(0,0);// ֹͣ�ƶ�
				wait = 0;
				break;
			}
			else
			{
				ChassisSpeed_Set(-yout,-xout);
			}
			PCModule_t.RecognitionModuleSte = RM_Identify;
		}
//		else if(wait_locXY > 50)//��ʱδ���յ����꣬������ͣ�ƶ�
//		{
//			ChassisSpeed_Set(0,0);// ֹͣ�ƶ�
//		}
		else if(wait == 1500)//���峬ʱ�����Զ�����ѭ����������
		{
			wait = 0;
			ChassisSpeed_Set(0,0);// ֹͣ�ƶ�
			break;
		}

	}
	RecognitionModule_Stop(&PCModule_t);
}
static void Color_Get(uint8_t mode)//ʶ����ɫ��ץȡ����һ��mode=0���ڶ���mode=1
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//����ͣ�ö�λϵͳ����
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

void RoughingArea_PutGet(uint8_t ColorSequenceNum)//�ּӹ����������ϣ�Բ����ɫ�̶������������
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//����ͣ�ö�λϵͳ����
	#endif
	uint8_t i;
		for(i=0; i<3; i++) //��ȡҪȡ��ɫ������
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
		for(i=0; i<3; i++) //��ȡҪȡ��ɫ������
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

void FashioningArea_Put(uint8_t ColorSequenceNum)//���ӹ����������ϣ�Բ����ɫ�̶������������
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//����ͣ�ö�λϵͳ����
	#endif
	uint8_t i;
	if(ColorSequenceNum == 0){
		for(i=0; i<3; i++) //���²�����
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
		for(i=0; i<3; i++) //�������
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


void RoughingArea_PutGet_R(uint8_t ColorSequenceNum)//�ּӹ����������ϣ�Բ����ɫ��������
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//����ͣ�ö�λϵͳ����
	#endif
	uint8_t i;
		for (i = 0; i < 3; i++) 
		{
			if(ColorSequence[ColorSequenceNum][i] == RED) {
					Get_1();
					if (CircleSequence[0] == RED) {
							Put_Blue(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == RED) {
							Put_Green(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == RED) {
							Put_Red(); // �Ŵ���������������Ȧ
					}
			}
			else if (ColorSequence[ColorSequenceNum][i] == GREEN) {
					Get_2();
					if (CircleSequence[0] == GREEN) {
							Put_Blue(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == GREEN) {
							Put_Green(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == GREEN) {
							Put_Red(); // �Ŵ���������������Ȧ
					}
			}
			else if (ColorSequence[ColorSequenceNum][i] == BLUE) {
					Get_3();
					if (CircleSequence[0] == BLUE) {
							Put_Blue(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == BLUE) {
							Put_Green(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == BLUE) {
							Put_Red(); // �Ŵ���������������Ȧ
					}
			}
		}
		//Debug_Await();
		for(i=0; i<3; i++) //��ȡҪȡ��ɫ������
		{
			if(ColorSequence[ColorSequenceNum][i] == RED){
				if (CircleSequence[0] == RED) {
						Get_Blue(); // �ô�����������һ��Ȧ
				}
				else if (CircleSequence[1] == RED) {
						Get_Green(); // �ô����������ڶ���Ȧ
				}
				else if (CircleSequence[2] == RED) {
						Get_Red(); // �ô���������������Ȧ
				}
				Put_1();
			}
			else if(ColorSequence[ColorSequenceNum][i] == GREEN){
				if (CircleSequence[0] == GREEN) {
						Get_Blue(); // �ô�����������һ��Ȧ
				}
				else if (CircleSequence[1] == GREEN) {
						Get_Green(); // �ô����������ڶ���Ȧ
				}
				else if (CircleSequence[2] == GREEN) {
						Get_Red(); // �ô���������������Ȧ
				}
				Put_2();
			}
			else if(ColorSequence[ColorSequenceNum][i] == BLUE){
				if (CircleSequence[0] == BLUE) {
						Get_Blue(); // �ô�����������һ��Ȧ
				}
				else if (CircleSequence[1] == BLUE) {
						Get_Green(); // �ô����������ڶ���Ȧ
				}
				else if (CircleSequence[2] == BLUE) {
						Get_Red(); // �ô���������������Ȧ
				}
				Put_3();
			}
		}
}
void FashioningArea_Put_R(uint8_t ColorSequenceNum)//���ӹ����������ϣ�Բ����ɫ��������
{
	#ifdef Disable_gyro_WhenArmMOVE
		ChassisRELAX_set();//����ͣ�ö�λϵͳ����
	#endif
	uint8_t i;
	if(ColorSequenceNum == 0){
		for (i = 0; i < 3; i++)//���²�����
		{
			if(ColorSequence[ColorSequenceNum][i] == RED) {
					
					if (CircleSequence[0] == RED) {
							Get_Blue(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == RED) {
							Get_Green(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == RED) {
							Get_Red(); // �Ŵ���������������Ȧ
					}
					Put_1();
			}
			else if (ColorSequence[ColorSequenceNum][i] == GREEN) {
					if (CircleSequence[0] == GREEN) {
							Get_Blue(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == GREEN) {
							Get_Green(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == GREEN) {
							Get_Red(); // �Ŵ���������������Ȧ
					}
					Put_2();
			}
			else if (ColorSequence[ColorSequenceNum][i] == BLUE) {
					if (CircleSequence[0] == BLUE) {
							Get_Blue(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == BLUE) {
							Get_Green(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == BLUE) {
							Get_Red(); // �Ŵ���������������Ȧ
					}
					Put_3();
			}
		}
	}

	if(ColorSequenceNum == 1){
		for (i = 0; i < 3; i++)//�������
		{
			if(ColorSequence[ColorSequenceNum][i] == RED) {
					
					if (CircleSequence[0] == RED) {
							Get_Blue(); // �Ŵ�����������һ��Ȧ
//							Get_Blue_Stacking(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == RED) {
							Get_Green(); // �Ŵ����������ڶ���Ȧ
//							Get_Green_Stacking(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == RED) {
							Get_Red(); // �Ŵ���������������Ȧ
//							Get_Red_Stacking(); // �Ŵ���������������Ȧ
					}
					Put_1();
			}
			else if (ColorSequence[ColorSequenceNum][i] == GREEN) {

					if (CircleSequence[0] == GREEN) {
							Get_Blue(); // �Ŵ�����������һ��Ȧ
//							Get_Blue_Stacking(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == GREEN) {
							Get_Green(); // �Ŵ����������ڶ���Ȧ
//							Get_Green_Stacking(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == GREEN) {
							Get_Red(); // �Ŵ���������������Ȧ
//							Get_Red_Stacking(); // �Ŵ���������������Ȧ
					}
					Put_2();
			}
			else if (ColorSequence[ColorSequenceNum][i] == BLUE) {

					if (CircleSequence[0] == BLUE) {
							Get_Blue(); // �Ŵ�����������һ��Ȧ
//							Get_Blue_Stacking(); // �Ŵ�����������һ��Ȧ
					}
					else if (CircleSequence[1] == BLUE) {
							Get_Green(); // �Ŵ����������ڶ���Ȧ
//							Get_Green_Stacking(); // �Ŵ����������ڶ���Ȧ
					}
					else if (CircleSequence[2] == BLUE) {
							Get_Red(); // �Ŵ���������������Ȧ
//							Get_Red_Stacking(); // �Ŵ���������������Ȧ
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
{//·���滮����
	RecognitionModule_Stop(&PCModule_t);
	Rest_action_S2();//�����ʼ̧��
	ChassisModle_Set(2);
	while(1)
	{
	#if 1
		Debug_Await();// �ȴ�����������
		Wait_Start();// �ȴ������ֳ�ʼ�����	
		//================����ɨ����================
		ChassisCoord_Set(0,-190, 0); 
		ChassisCoord_WaitStop();
		ChassisCoord_Set(-750,-190, 0);
		QR_action();//��ά��ʶ����
		ChassisCoord_WaitStop();
		//===============ʶ���ά��===============
		Identify_QRCode();
		//==============ǰ����ԭ����==============
		//===============���м�===============
		ChassisCoord_Set(-1080, -180, 180);
		ChassisCoord_WaitStop();
		//=============�ƶ����ּӹ���=============��������������������������������������������������������
		ChassisCoord_Set(-1080, -1905, 0);
		IdentifyCircle_action();
		ChassisCoord_WaitStop();
		
		IdentifyCircle_Get(100,150);//�Ӿ�ʶ��Բ��
		Get_color();    //ͬʱ�ܹ���ɫ��������˳��Ļ�е�۶����Ƕ�
		Identify_circle_Seq();//ʶ��Բ����ɫ˳��
		FashioningArea_Put_R(0); //ɫ������ּӹ�����
		SetCoordinateXY(0,0);//���ö�λϵͳ������ϵ��������ǰ���
		Rest_action_S2();
		
		ChassisCoord_Set(-850, 0, -90);
		ChassisCoord_WaitStop();
		//=============�ƶ������ӹ���=============��������������������������������������������������������
		ChassisCoord_Set(-848, 828, 0);
		ChassisCoord_WaitStop();
		IdentifyCircle_action();
		
		
		IdentifyCircle_Get(100,150);//�Ӿ�ʶ��Բ��
		Rest_action_S2();//��λ����
		RoughingArea_PutGet(0);
		SetCoordinateXY(0,0);//���ö�λϵͳ������ϵ��������ǰ���
		
		ChassisCoord_Set(0, 885, -90);
		ChassisCoord_WaitStop();
		ChassisCoord_Set(460, 885,0);
		ChassisCoord_WaitStop();
		
		IdentifyColor_action();
		Color_Get(0);
		
		IdentifyColor_Get(0);//�����Ϸŵ�ת��
		
		SetCoordinateXY(-1530,-180);//���ö�λϵͳת��λ��
		
		//===============���м�===============
		ChassisCoord_Set(-1090, -180, 180);
		ChassisCoord_WaitStop();	
		
		//=============�ƶ����ּӹ���=============��������������������������������������������������������
		ChassisCoord_Set(-1090, -1905, 0);
		IdentifyCircle_action();
		ChassisCoord_WaitStop();
		
		IdentifyCircle_Get(100,150);//�Ӿ�ʶ��Բ��
		Get_color();    //ͬʱ�ܹ���ɫ��������˳��Ļ�е�۶����Ƕ�
		Identify_circle_Seq();//ʶ��Բ����ɫ˳��
		FashioningArea_Put_R(1); //ɫ������ּӹ�����
		SetCoordinateXY(0,0);//���ö�λϵͳ������ϵ��������ǰ���
		Rest_action_S2();
		
		ChassisCoord_Set(-850, 0, -90);
		ChassisCoord_WaitStop();
		//=============�ƶ������ӹ���=============��������������������������������������������������������
		ChassisCoord_Set(-850, 828, 0);
		IdentifyCircle_action();
		ChassisCoord_WaitStop();
		
		IdentifyCircle_Get(100,150);//�Ӿ�ʶ��Բ��
		Rest_action_S2();//��λ����
		RoughingArea_PutGet(1);
		SetCoordinateXY(0,0);//���ö�λϵͳ������ϵ��������ǰ���
		
		ChassisCoord_Set(0, 885, -90);
		ChassisCoord_WaitStop();
		ChassisCoord_Set(420, 897,0);
		ChassisCoord_WaitStop();
		
		IdentifyColor_action();
		Color_Get(0);
		
		IdentifyColor_Get(1);//�����Ϸŵ�ת��
		SetCoordinateXY(-1530,-180);//���ö�λϵͳת��λ��
//===============�س�����===============
		ChassisCoord_Set(-720,-190, 0);
		ChassisCoord_WaitStop();
		ChassisCoord_Set(0,-190, 0); 
		ChassisCoord_WaitStop();
		ChassisCoord_Set(0,0, 0); 
		ChassisCoord_WaitStop();
		
		


		
	#else
		Debug_Await();// �ȴ�����������
		Wait_Start();// �ȴ������ֳ�ʼ�����
		
		IdentifyColor_Get(0);//�����Ϸŵ�ת��
		
		
		
	
	
		#endif
		//*/
		Debug_Await();// �ȴ���һ������
	}
}
int Task_User_create(void)
{
	thread_PathWrite = rt_thread_create( "PathWrite",             /* �߳����� */
								         PathWrite_task,          /* �߳���ں��� */
								         RT_NULL,                 /* �߳���ں������� */
								         1024,          		  /* �߳�ջ��С */
								         10,                      /* �̵߳����ȼ� */
								         20);                     /* �߳�ʱ��Ƭ */
	if(thread_PathWrite != RT_NULL)
	{
		rt_thread_startup(thread_PathWrite);
		rt_kprintf("thread_PathWrite startup!\n");
	}
	
	return 0;
}
