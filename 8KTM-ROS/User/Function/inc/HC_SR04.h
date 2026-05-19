#ifndef __HC_SR04_H
#define __HC_SR04_H


#include "mytype.h" 



#define HCSR04_TRIG_HIGH 1
#define HCSR04_TRIG_LOW  0
#define HCSR04_TIME_MAX 3000
#define HCSR04_TEMP  28.0f      //温度
#define HCSR04_V ( (0.305f * HCSR04_TEMP) +165.7f )         //声音速度 
#define HCSR04_L2 2.7f  //单位cm L的平方

#define N 5

#define HC_SR04_NUM 4

typedef struct 
{
	uint8_t enabled;
	uint8_t PulseOverflowCnt;
	uint8_t count;
	enum {leisure=0,err,success,start,await_RISING,await_FALLING} sta;  //状态  
	uint16_t time; //时间
	uint32_t TIMECNT;
	float PulseWidthValue;
	float DistanceValue;  //距离值
	float H;
	float H_buf[N];
	float H_sort[N];
	float H_sum;
	uint8_t (*HCSR04_Echo)(void);  //回波引脚
	void (*HCSR04_Trig)(u8 mode);  //脉冲触发引脚
}HCSR04DataTypedef;

extern HCSR04DataTypedef sHCSR04Data[HC_SR04_NUM];

void HCSR04_Inti( HCSR04DataTypedef *handle,
									uint8_t   (*HCSR04_Echo)(void),
								  void (*HCSR04_Trig)(u8 mode), 
				          uint8_t enabled);
void HCSR04Work_Scan(HCSR04DataTypedef *handle);//10us刷一次 
void HCSR04_PulseWidthRead(HCSR04DataTypedef *handle);
uint8_t HCSR04_DistanceCalc(HCSR04DataTypedef *handle,uint16_t *data);                    
void HCSR04_TimeOverflow(HCSR04DataTypedef *handle);
void HCSR04_Start(HCSR04DataTypedef *handle);
uint8_t HCSR04_ReadState(HCSR04DataTypedef *handle);
void HCSR04_Enabled(HCSR04DataTypedef *handle,uint8_t enabled);			   
void HC_SR04_Handler(void);				   
				   
#endif

