#include "data.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "Servo.h"
#include "thread_rccu.h"
#include "SCServo.h"
#include "SLAVE_SteeringEngine_3CH/SLAVE_SteeringEngine_3CH.h"
typedef struct{
	int16_t Num1;//舵机ID
	int16_t Num2;
	int16_t Num3;
	int16_t Num4;
}Servo;
//int16_t Red_1 = 1912;
//int16_t Green_1 = 3293;
//int16_t Blue_1 = 582;


//int16_t Open_Claw = 800;
//int16_t Mid_Claw = 1100;
//int16_t Close_Claw = 1370;
int16_t Open_Claw = 2300;
int16_t Mid_Claw = 2000;
int16_t Close_Claw = 1600;
/* ------------------------------------------------------------*/
Servo bowl_1 		= {//1号碗
								787,
								2065,
								2080,//>>>>>2050
								};
Servo bowl_2 		= {//2号碗
								1031,
								2037,
								2084,//>>>>>2050
								};
Servo bowl_3 		= {//3号碗
								1248,
								2093,
								2102,//>>>>>2050
								};
/* ------------------------------------------------------------*/
Servo Red 		= {//红一层
								3502,
								2864,
								1812
								};
Servo Green 	= {//绿一层
								3095,
								2686,
								1606
								};
Servo Blue 		= {//蓝一层
								2669,
								2840,
								1773
								};
/* ------------------------------------------------------------*/
Servo Red_Stacking 		= {//红码垛
								3510,
								2593,
								1989	
								};
Servo Green_Stacking 	= {//绿码垛
								3100,
								2390,
								1882
								};
Servo Blue_Stacking 	= {//蓝码垛
								2660,
								2565,
								2016
								};
/* ------------------------------------------------------------*/

		void Rest_action()//归位动作
		{
			WritePosEx( 2, 1744, 3250, 160);
			WritePosEx( 3, 2501, 3250, 160);
			My_mDelay(300);
			
		}
		void Rest_action_S2()//初始动作
		{
			WritePosEx(2, 2140, 4250, 180);		//2号舵机控制前后
			WritePosEx(3, 2280, 3250, 160);		//3号舵机控制上下
			My_mDelay(200);
			WritePosEx(1, 1040, 3000,160);	
			WritePosEx(4, Open_Claw, 3250, 160);		//4号舵机手爪
			My_mDelay(500);
		}
		
		void QR_action()//识别二维码动作
		{
			WritePosEx( 1, 3073, 3250, 160);
			My_mDelay(500);
			WritePosEx( 2, 1738, 3250, 160);
			WritePosEx( 3, 2567, 3250, 160);
			WritePosEx(4, Open_Claw, 3250, 160);		//4号舵机手爪
			My_mDelay(300);
		}
		
		void IdentifyColor_action()//识别原料颜色动作
		{
			WritePosEx(1, 2936, 3000,160);
			My_mDelay(500);
			WritePosEx(2, 2237, 3250, 160);
			WritePosEx(3, 2334, 3250, 160);
//			WritePosEx(4, Open_Claw, 3250, 160);
			My_mDelay(600);
		}
		void IdentifyCircle_action()//粗加工区识别圆环动作
		{
			WritePosEx(1, 2900, 3000,160);
			My_mDelay(400);
			WritePosEx(2, 2226, 3250, 160);
			WritePosEx(3, 2228, 3250, 160);//2154
			WritePosEx(4, Mid_Claw, 3250, 160);
			My_mDelay(200);
		}
//		void Get_A()//抓取原料区物料
//		{
//			WritePosEx(1, 3136, 2500,60);
//			WritePosEx(2, 2263, 3250, 60);
//			WritePosEx(3, 1920, 3250, 60);
//			My_mDelay(500);
//			WritePosEx(4, Close_Claw,  3250, 60);//抓
//			My_mDelay(300);
//			Rest_action();//归位
//		}
		void Get_A()//抓取原料区物料
		{
			WritePosEx(4, Open_Claw, 4250, 120);
			My_mDelay(200);
			WritePosEx(1, 3102, 3000,160);
			WritePosEx(2, 2410, 3250, 160);
			WritePosEx(3, 1938, 3250, 160);
			My_mDelay(500);
			WritePosEx(4, Close_Claw,  4250, 180);//抓
			My_mDelay(500);
			Rest_action();//归位
		}
		void Get_B()//抓取原料区物料
		{
			WritePosEx(1, 3136, 3000,160);
			WritePosEx(2, 2263, 3250, 160);
			WritePosEx(3, 1920, 3250, 160);
			My_mDelay(500);
			WritePosEx(4, Close_Claw,  3250, 160);//抓
			My_mDelay(500);
			Rest_action();//归位
		}
		
void Put_1()//放到1号碗（红）
		{
			WritePosEx(1, bowl_1.Num1, 3000,160);//转到1号碗方向
			My_mDelay(900);
			WritePosEx(2, bowl_1.Num2, 3250, 160);//向前伸
			My_mDelay(200);
			WritePosEx(3, bowl_1.Num3, 3250, 160);//向下伸
			My_mDelay(300);
			WritePosEx(4, Open_Claw, 3250, 160);//放
			My_mDelay(500);
			WritePosEx(3, 2501, 3250, 160);
			My_mDelay(200);
		}
		void Put_2()//放到2号碗（绿）
		{
			WritePosEx(1, bowl_2.Num1, 3000,160);//转到2号碗方向
			My_mDelay(600);
			WritePosEx(2, bowl_2.Num2, 3250, 160);//向前伸
			My_mDelay(200);
			WritePosEx(3, bowl_2.Num3, 3250, 160);//向下伸
			My_mDelay(300);
			WritePosEx(4, Open_Claw, 3250, 160);//放
			My_mDelay(500);
			WritePosEx(3, 2501, 3250, 160);
			My_mDelay(200);
		}
		void Put_3()//放到3号碗（蓝）
		{
			WritePosEx(1, bowl_3.Num1, 3000,160);//转到3号碗方向
			My_mDelay(400);
			WritePosEx(2, bowl_3.Num2, 3250, 160);//向前伸
			My_mDelay(200);
			WritePosEx(3, bowl_3.Num3, 3250, 160);//向下伸	
			My_mDelay(300);
			WritePosEx(4, Open_Claw, 3250, 160);//放
			My_mDelay(500);
		}
		void Get_1()//抓1号碗物料（红）
		{

			WritePosEx(1, bowl_1.Num1, 3000,160);//转到1号碗方向
			My_mDelay(800);
			WritePosEx(4, Mid_Claw, 3250, 160);//放
			WritePosEx(2, bowl_1.Num2, 3250, 160);//向前伸
			WritePosEx(3, bowl_1.Num3, 3250, 160);//向下伸
			My_mDelay(200);
			WritePosEx(4, Close_Claw, 4250, 120);//抓
			My_mDelay(500);
			WritePosEx(3, 2382, 3250, 160);//向上伸			
			My_mDelay(200);
			Rest_action();//归位
		}
		void Get_2()//抓2号碗物料（绿）
		{

			WritePosEx(1, bowl_2.Num1, 3000,160);//转到2号碗方向
			My_mDelay(600);
			WritePosEx(4, Mid_Claw, 3250, 160);//放
			WritePosEx(2, bowl_2.Num2, 3250, 160);//向前伸		
			WritePosEx(3, bowl_2.Num3, 3250, 160);//向下伸			
			My_mDelay(300);
			WritePosEx(4, Close_Claw, 4250, 120);//抓
			My_mDelay(500);
			WritePosEx(3, 2382, 3250, 160);//向上伸			
			My_mDelay(200);
			Rest_action();//归位
		}
		void Get_3()//抓3号碗物料（蓝）
		{

			WritePosEx(1, bowl_3.Num1, 3000,160);//转到3号碗方向
			My_mDelay(600);
			WritePosEx(4, Mid_Claw, 3250, 160);//放
			WritePosEx(2, bowl_3.Num2, 3250, 160);//向前伸		
			WritePosEx(3, bowl_3.Num3, 3250, 160);//向下伸		
			My_mDelay(300);
			WritePosEx(4, Close_Claw, 4250, 120);//抓
			My_mDelay(500);
			WritePosEx(3, 2382, 3250, 160);//向上伸
			My_mDelay(200);
			Rest_action();//归位
		}
		void Put_Red()//放红环——第一层
		{
			WritePosEx(1, Red.Num1, 3000,160);//转到红环方向
			My_mDelay(1000);
			WritePosEx(2, Red.Num2, 3250, 160);
			WritePosEx(3, Red.Num3, 3250, 160);
			My_mDelay(300);
			WritePosEx(4, Open_Claw, 4250, 160);//爪子松开
			My_mDelay(500);//300
			Rest_action();
		}
		void Put_Green()//放绿环——第一层
		{
			WritePosEx(1, Green.Num1, 3000,160);//转到绿环方向
			My_mDelay(800);
			WritePosEx(3, Green.Num3, 3250, 160);
			WritePosEx(2, Green.Num2, 3250, 160);//向前伸
			My_mDelay(300);
			WritePosEx(4, Open_Claw, 4250, 160);//爪子松开
			My_mDelay(500);
			Rest_action();
		}
		void Put_Blue()//放蓝环——第一层
		{
			WritePosEx(1, Blue.Num1, 3000,160);//转到蓝环方向
			My_mDelay(600);
			WritePosEx(2, Blue.Num2, 3250, 160);//向前伸
			WritePosEx(3, Blue.Num3, 3250, 160);
			My_mDelay(300);
			WritePosEx(4, Open_Claw, 4250, 160);//爪子松开
			My_mDelay(500);
			Rest_action();
		}
		
		void Put_Red_Stacking()//放红环——码垛
		{
			WritePosEx(1, Red_Stacking.Num1, 3000,160);//转到红环方向
			My_mDelay(1000);
			WritePosEx(2, Red_Stacking.Num2, 3250, 160);//向前伸
			WritePosEx(3, Red_Stacking.Num3, 3250, 160);
			My_mDelay(500);
			WritePosEx(4, Open_Claw, 4250, 160);//爪子松开
			My_mDelay(500);//300
			Rest_action();
		}
		void Put_Green_Stacking()//放绿环——码垛
		{
			WritePosEx(1, Green_Stacking.Num1, 3000,160);//转到绿环方向
			My_mDelay(800);
			WritePosEx(3, Green_Stacking.Num3, 3250, 160);
			WritePosEx(2, Green_Stacking.Num2, 3250, 160);//向前伸
			My_mDelay(500);
			WritePosEx(4, Open_Claw, 4250, 160);//爪子松开
			My_mDelay(500);
			Rest_action();
		}
		void Put_Blue_Stacking()//放蓝环——码垛
		{
			WritePosEx(1, Blue_Stacking.Num1, 3000,160);//转到蓝环方向
			My_mDelay(600);
			WritePosEx(2, Blue_Stacking.Num2, 3250, 160);//向前伸
			WritePosEx(3, Blue_Stacking.Num3, 3250, 160);
			My_mDelay(500);
			WritePosEx(4, Open_Claw, 4250, 160);//爪子松开
			My_mDelay(500);
			Rest_action();
		}
		void Get_Red()//抓红环物料
		{
			WritePosEx(1, Red.Num1, 3000,160);//转到红环方向
			My_mDelay(800);
			WritePosEx(2, Red.Num2, 3250, 160);//向前伸
			WritePosEx(3, Red.Num3, 3250, 160);
			My_mDelay(300);
			WritePosEx(4, Close_Claw, 3250, 160);//抓
			My_mDelay(500);
			Rest_action();
		}
		void Get_Green()//抓绿环物料
		{
			WritePosEx(1, Green.Num1, 3000,160);//转到绿环方向
			My_mDelay(600);
			WritePosEx(3, Green.Num3, 3250, 160);
			WritePosEx(2, Green.Num2, 3250, 160);//向前伸
			My_mDelay(300);
			WritePosEx(4, Close_Claw, 3250, 160);//抓
			My_mDelay(500);
			Rest_action();
		}
		void Get_Blue()//抓蓝环物料
		{
			WritePosEx(1, Blue.Num1, 3000,160);//转到蓝环方向
			My_mDelay(400);
			WritePosEx(2, Blue.Num2, 3250, 160);//向前伸
			WritePosEx(3, Blue.Num3, 3250, 160);			
			My_mDelay(300);
			WritePosEx(4, Close_Claw, 3250, 160);//抓
			My_mDelay(500);
			Rest_action();
		}
		
		void Get_Red_Stacking()//抓红环物料
		{
			WritePosEx(4, 2047, 2250, 30);//爪子松开
			HAL_Delay(100);
			WritePosEx(1, Red_Stacking.Num1, 2250, 30);//转到红环方向
			HAL_Delay(1500);
			WritePosEx(2, Red_Stacking.Num2, 2250, 30);//向前伸
			WritePosEx(3, Red_Stacking.Num3, 2250, 30);
			RegWriteAction();
			HAL_Delay(1200);
			WritePosEx(4, 1600, 0, 0);//爪子抓紧
			HAL_Delay(500);
			Rest_action();
			HAL_Delay(500);
		}
		void Get_Green_Stacking()//抓绿环物料
		{
			WritePosEx(4, 2047, 2250, 30);//爪子松开
			HAL_Delay(100);
			WritePosEx(1, Green_Stacking.Num1, 2250, 30);//转到绿环方向
			HAL_Delay(1500);
			WritePosEx(3, Green_Stacking.Num3, 2250, 30);
			WritePosEx(2, Green_Stacking.Num2, 2250, 30);//向前伸
			RegWriteAction();
			HAL_Delay(1200);
			WritePosEx(4, 1600, 0, 0);//爪子抓紧
			HAL_Delay(500);
			Rest_action();
			HAL_Delay(500);
		}
		void Get_Blue_Stacking()//抓蓝环物料
		{
			WritePosEx(4, 2047, 2250, 30);//爪子松开
			HAL_Delay(100);
			WritePosEx(1, Blue_Stacking.Num1, 2250, 30);//转到蓝环方向
			HAL_Delay(1500);
			RegWritePosEx(2, Blue_Stacking.Num2, 2250, 30);//向前伸
			RegWritePosEx(3, Blue_Stacking.Num3, 2250, 30);
			RegWriteAction();
			HAL_Delay(1200);
			WritePosEx(4, 1600, 0, 0);//爪子抓紧
			HAL_Delay(500);
			Rest_action();
			HAL_Delay(500);
		}
		void Get_color()//决赛识别粗加工物料顺序动作
		{
			WritePosEx(1, 3252, 3000,160);
			WritePosEx(2, 1957, 3250, 160);
			WritePosEx(3, 2363, 3250, 160);
			My_mDelay(500);
			WritePosEx(4, Open_Claw,  3250, 160);//抓
			My_mDelay(500);
		}
		void Put_A()//放置原料区物料
		{
			WritePosEx(1, 3102, 3000, 160);
			My_mDelay(300);
			WritePosEx(2, 2350, 3250, 160);
			WritePosEx(3, 1938, 3250, 160);
			My_mDelay(400);
			WritePosEx(4, Open_Claw, 4250, 160);//放
			My_mDelay(500);
			Rest_action();//归位
		}
		

		
		void Put_B()//放置原料区物料码垛
		{
			WritePosEx(1, 3102, 3000, 160);
			My_mDelay(300);
			WritePosEx(2, 2350, 3250, 160);
			WritePosEx(3, 1938, 3250, 160);
			My_mDelay(400);
			WritePosEx(4, Open_Claw, 4250, 160);//放
			My_mDelay(500);
			Rest_action();//归位
		}