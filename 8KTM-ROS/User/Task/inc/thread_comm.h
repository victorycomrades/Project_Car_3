#ifndef __THREAD_COMM_H_
#define __THREAD_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EncodingWheel	//是否使用编码轮
//#define HWT101_gyro 	//是否使用副陀螺仪
#define Disable_gyro_WhenArmMOVE //机械臂移动时关闭陀螺仪

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
extern float pos_x;
extern float pos_y;
extern float zangle;
extern int StartTemperature;
extern bool CalibratResults;
extern int Wheel_isReady;




#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __THREAD_COMM_H_ */


