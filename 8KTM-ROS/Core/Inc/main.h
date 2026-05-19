/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define POWER_Pin GPIO_PIN_2
#define POWER_GPIO_Port GPIOE
#define POWER_KEY_Pin GPIO_PIN_3
#define POWER_KEY_GPIO_Port GPIOE
#define KEY_Pin GPIO_PIN_4
#define KEY_GPIO_Port GPIOE
#define IN1_Pin GPIO_PIN_0
#define IN1_GPIO_Port GPIOC
#define IN2_Pin GPIO_PIN_1
#define IN2_GPIO_Port GPIOC
#define IN3_Pin GPIO_PIN_2
#define IN3_GPIO_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define DC_Pin GPIO_PIN_6
#define DC_GPIO_Port GPIOA
#define LCD_RESET_Pin GPIO_PIN_4
#define LCD_RESET_GPIO_Port GPIOC
#define W25Qxx_CS_Pin GPIO_PIN_0
#define W25Qxx_CS_GPIO_Port GPIOB
#define KEY5_Pin GPIO_PIN_7
#define KEY5_GPIO_Port GPIOE
#define KEY4_Pin GPIO_PIN_8
#define KEY4_GPIO_Port GPIOE
#define KEY3_Pin GPIO_PIN_9
#define KEY3_GPIO_Port GPIOE
#define KEY2_Pin GPIO_PIN_10
#define KEY2_GPIO_Port GPIOE
#define KEY1_Pin GPIO_PIN_11
#define KEY1_GPIO_Port GPIOE
#define QB_Pin GPIO_PIN_12
#define QB_GPIO_Port GPIOE
#define VRS1_Pin GPIO_PIN_13
#define VRS1_GPIO_Port GPIOE
#define VRS2_Pin GPIO_PIN_14
#define VRS2_GPIO_Port GPIOE
#define VRS3_Pin GPIO_PIN_15
#define VRS3_GPIO_Port GPIOE
#define IR_2_Pin GPIO_PIN_14
#define IR_2_GPIO_Port GPIOB
#define IR_1_Pin GPIO_PIN_15
#define IR_1_GPIO_Port GPIOB
#define TRIG1_Pin GPIO_PIN_8
#define TRIG1_GPIO_Port GPIOD
#define TRIG2_Pin GPIO_PIN_9
#define TRIG2_GPIO_Port GPIOD
#define TRIG3_Pin GPIO_PIN_10
#define TRIG3_GPIO_Port GPIOD
#define TRIG4_Pin GPIO_PIN_11
#define TRIG4_GPIO_Port GPIOD
#define ECHO1_Pin GPIO_PIN_12
#define ECHO1_GPIO_Port GPIOD
#define ECHO1_EXTI_IRQn EXTI15_10_IRQn
#define ECHO2_Pin GPIO_PIN_13
#define ECHO2_GPIO_Port GPIOD
#define ECHO2_EXTI_IRQn EXTI15_10_IRQn
#define ECHO3_Pin GPIO_PIN_14
#define ECHO3_GPIO_Port GPIOD
#define ECHO3_EXTI_IRQn EXTI15_10_IRQn
#define ECHO4_Pin GPIO_PIN_15
#define ECHO4_GPIO_Port GPIOD
#define ECHO4_EXTI_IRQn EXTI15_10_IRQn
#define MAX3485_EN_Pin GPIO_PIN_8
#define MAX3485_EN_GPIO_Port GPIOC
#define RS485_EN_Pin GPIO_PIN_15
#define RS485_EN_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_3
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_4
#define LED2_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
