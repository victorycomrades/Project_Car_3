/* 包含头文件 ----------------------------------------------------------------*/
#include "HC_SR04.h"
#include "string.h"
#include <math.h>
/* 私有类型定义 --------------------------------------------------------------*/

/* 私有宏定义 ----------------------------------------------------------------*/

/* 私有变量 ------------------------------------------------------------------*/


/* 扩展变量 ------------------------------------------------------------------*/
HCSR04DataTypedef sHCSR04Data[HC_SR04_NUM];

extern TIM_HandleTypeDef htim2;

/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

//冒泡排序
static void bubble_sort(float a[], int n)
{
	int i, j;
	float temp;
	
    for (j = 0; j < n - 1; j++)
			for (i = 0; i < n - 1 - j; i++)
			{
				if(a[i] > a[i + 1])
				{
					temp = a[i];
					a[i] = a[i + 1];
					a[i + 1] = temp;
				}
			}
}
void HCSR04_Inti( HCSR04DataTypedef *handle,
                   uint8_t   (*HCSR04_Echo)(void),
                   void (*HCSR04_Trig)(u8 mode), 
				   uint8_t enabled)
{
	memset(handle, 0, sizeof(HCSR04DataTypedef));
	handle->sta = leisure;  //状态  
	handle->HCSR04_Echo = HCSR04_Echo;  //回波引脚
	handle->HCSR04_Trig = HCSR04_Trig;  //脉冲触发引脚
	handle->enabled = (enabled != 0);
}
void HCSR04Work_Scan(HCSR04DataTypedef *handle)//10us刷一次
{
	if(handle->sta > success)
	{
		if(handle->time == 0)//拉高Trig
		{
			handle->HCSR04_Trig(HCSR04_TRIG_HIGH);
		}
		else if(handle->time == 1)//拉低Trig
		{
			handle->HCSR04_Trig(HCSR04_TRIG_LOW);
			handle->sta = await_RISING; //等待上升沿
		}
		handle->time++;
		if(handle->time >= HCSR04_TIME_MAX)
		{
			handle->time = 0;
			if(handle->sta == await_RISING)
			{
				HAL_TIM_Base_Stop_IT(&htim2);
				handle->sta = err;
				handle->count = 0;
			}

		}
	}
}
void HCSR04_PulseWidthRead(HCSR04DataTypedef *handle)
{
	if(handle->HCSR04_Echo() == 1) //上升沿
	{
		if(handle->sta == await_RISING)
		{
			htim2.Instance->CNT = 0;
			handle->PulseOverflowCnt = 0;
			HAL_TIM_Base_Start_IT(&htim2);
			handle->sta = await_FALLING;
		}
	}
	else if(handle->sta == await_FALLING)
	{
		handle->TIMECNT = htim2.Instance->CNT;
		HAL_TIM_Base_Stop_IT(&htim2);
		handle->sta = success;
		handle->time = 0;
	}
}
uint8_t HCSR04_DistanceCalc(HCSR04DataTypedef *handle,uint16_t *data)
{
	if(success == handle->sta)
	{
		handle->PulseWidthValue = ((handle->PulseOverflowCnt*65535)+handle->TIMECNT)/28000000.0f;
		handle->DistanceValue = HCSR04_V*handle->PulseWidthValue*100; //换算到cm
		handle->H_buf[handle->count++] = sqrt(handle->DistanceValue*handle->DistanceValue - HCSR04_L2);
		if(handle->count == N)
		{
			for(uint8_t i =0; i<N; i++)
			{
				handle->H_sort[i] = handle->H_buf[i];
			}
			bubble_sort(handle->H_sort,N);
			handle->H_sum = handle->H_sort[(N-1)/2] + handle->H_sort[((N-1)/2)+1]+ handle->H_sort[((N-1)/2)-1];;
			handle->H = handle->H_sum/3.0f;
			handle->count = 0;
			handle->sta = leisure;
			*data = (handle->H+0.05f)*10;
			return 1;
		}
		else
		{
			handle->time = HCSR04_TIME_MAX - 200;
			handle->sta = start;
		}
	
	}
	return 0;
}
void HCSR04_TimeOverflow(HCSR04DataTypedef *handle)
{
	if(await_FALLING == handle->sta)
	{
		handle->PulseOverflowCnt++;
		if(handle->PulseOverflowCnt > 5)
		{
			HAL_TIM_Base_Stop_IT(&htim2);
			handle->sta = err;
			handle->time = 0;
			handle->count = 0;
		}
	}
}

void HCSR04_Start(HCSR04DataTypedef *handle)
{
	if((handle->sta < success) && (handle->enabled == 1))
	{
		handle->time = 0;
		handle->count = 0;
		handle->PulseOverflowCnt = 0;
		handle->sta = start;
	}
}

uint8_t HCSR04_ReadState(HCSR04DataTypedef *handle)
{
	if(handle->sta < success)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void HCSR04_Enabled(HCSR04DataTypedef *handle,uint8_t enabled)
{
	handle->enabled = (enabled != 0);
}

void HC_SR04_Handler(void)
{//定时中断1ms
	static uint8_t ch = 0;
	static uint8_t ch_old = 0;
	
	if((HCSR04_ReadState(&sHCSR04Data[ch]) == 1) && (HCSR04_ReadState(&sHCSR04Data[ch_old]) == 1))
	{
		HCSR04_Start(&sHCSR04Data[ch]);
		ch_old = ch;
		ch++;
		if(ch == HC_SR04_NUM) 
		{
			ch = 0;
		}
	}
}
