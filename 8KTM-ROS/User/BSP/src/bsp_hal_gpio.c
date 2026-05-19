/* Includes ------------------------------------------------------------------*/
#include "bsp_hal_gpio.h"
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static void (*EXTICallBackFunc)(uint16_t GPIO_Pin) = NULL;
/* Private functions ---------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
#ifdef LED1_GPIO_Port
void led1_level(uint8_t level)
{
    if (level)
        LED1_GPIO_Port->BSRR = LED1_Pin;
    else
        LED1_GPIO_Port->BSRR = (uint32_t)LED1_Pin << 16u;
}
#endif
#ifdef LED2_GPIO_Port
void led2_level(uint8_t level)
{
    if (level)
        LED2_GPIO_Port->BSRR = LED2_Pin;
    else
        LED2_GPIO_Port->BSRR = (uint32_t)LED2_Pin << 16u;
}
#endif
#ifdef LED3_GPIO_Port
void led3_level(uint8_t level)
{
    if (level)
        LED3_GPIO_Port->BSRR = LED3_Pin;
    else
        LED3_GPIO_Port->BSRR = (uint32_t)LED3_Pin << 16u;
}
#endif
#ifdef KEY1_GPIO_Port
uint8_t KEY_1(void)
{
    if ((KEY1_GPIO_Port->IDR & KEY1_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
#ifdef KEY2_GPIO_Port
uint8_t KEY_2(void)
{
    if ((KEY2_GPIO_Port->IDR & KEY2_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
#ifdef KEY3_GPIO_Port
uint8_t KEY_3(void)
{
    if ((KEY3_GPIO_Port->IDR & KEY3_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
#ifdef KEY4_GPIO_Port
uint8_t KEY_4(void)
{
    if ((KEY4_GPIO_Port->IDR & KEY4_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
#ifdef KEY5_GPIO_Port
uint8_t KEY_5(void)
{
    if ((KEY5_GPIO_Port->IDR & KEY5_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
#ifdef KEY6_GPIO_Port
uint8_t KEY_6(void)
{
    if ((KEY6_GPIO_Port->IDR & KEY6_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif
#ifdef KEY7_GPIO_Port
uint8_t KEY_7(void)
{
    if ((KEY7_GPIO_Port->IDR & KEY7_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

uint8_t Echo1_read(void)
{
    if ((ECHO1_GPIO_Port->IDR & ECHO1_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
uint8_t Echo2_read(void)
{
    if ((ECHO2_GPIO_Port->IDR & ECHO2_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
uint8_t Echo3_read(void)
{
    if ((ECHO3_GPIO_Port->IDR & ECHO3_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
uint8_t Echo4_read(void)
{
    if ((ECHO4_GPIO_Port->IDR & ECHO4_Pin) != (uint32_t)GPIO_PIN_RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void Trig1_level(uint8_t level)
{
    GPIO_PinState PinState;
    if(level)
    {
        PinState = GPIO_PIN_SET;
    }
    else
    {
        PinState = GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(TRIG1_GPIO_Port, TRIG1_Pin, PinState);
}
void Trig2_level(uint8_t level)
{
    GPIO_PinState PinState;
    if(level)
    {
        PinState = GPIO_PIN_SET;
    }
    else
    {
        PinState = GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(TRIG2_GPIO_Port, TRIG2_Pin, PinState);
}
void Trig3_level(uint8_t level)
{
    GPIO_PinState PinState;
    if(level)
    {
        PinState = GPIO_PIN_SET;
    }
    else
    {
        PinState = GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(TRIG3_GPIO_Port, TRIG3_Pin, PinState);
}
void Trig4_level(uint8_t level)
{
    GPIO_PinState PinState;
    if(level)
    {
        PinState = GPIO_PIN_SET;
    }
    else
    {
        PinState = GPIO_PIN_RESET;
    }
    HAL_GPIO_WritePin(TRIG4_GPIO_Port, TRIG4_Pin, PinState);
}

void EXTI_SetHandler(void (*Func)(uint16_t GPIO_Pin))  
{
	EXTICallBackFunc = Func;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) 
{
	if(EXTICallBackFunc != NULL)
	{
		EXTICallBackFunc(GPIO_Pin);
	}
}














