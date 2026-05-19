/* 包含头文件 ----------------------------------------------------------------*/
#include "bsp_hal_time.h"

/* 私有类型定义 --------------------------------------------------------------*/
#define TIMER htim4
/* 私有宏定义 ----------------------------------------------------------------*/

/* 私有变量 ------------------------------------------------------------------*/
static VoidFuncVoid TimeCallBackFunc = NULL;
static VoidFuncVoid SysTickCallBackFunc = NULL;
static VoidFuncVoid Time2UpdateCallBackFunc = NULL;
/* 扩展变量 ------------------------------------------------------------------*/

/* 私有函数原形 --------------------------------------------------------------*/

/* 函数体 --------------------------------------------------------------------*/
/***************定时中断初始化************************/
void Timer_SetHandler(VoidFuncVoid func)  //1ms的定时中断
{
	TimeCallBackFunc = func;
	HAL_TIM_Base_Start_IT(&TIMER);
}
void SysTick_SetHandler(VoidFuncVoid func)  //1ms的定时中断
{
	SysTickCallBackFunc = func;
}
void Timer2Update_SetHandler(VoidFuncVoid func)
{
	Time2UpdateCallBackFunc = func;
}
/***********TIM溢出中断****************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//HAL
{
	if(TIMER.Instance == htim->Instance)
	{
		if(NULL != TimeCallBackFunc)
		{
			TimeCallBackFunc();
		}
	}
	else if(htim2.Instance == htim->Instance)
	{
		if(NULL != Time2UpdateCallBackFunc)
		{
			Time2UpdateCallBackFunc();
		}
	}
}

