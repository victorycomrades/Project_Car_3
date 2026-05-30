#ifndef __THREAD_RCCU_H_
#define __THREAD_RCCU_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* Includes ------------------------------------------------------------------*/
#include "mytype.h"
#include "chassis_LineTracker.h"
#include "Location_Tracker.h"
#include "chassis_function.h"
#include "pid.h"
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum
{
    CHASSIS_RELAX = 0,
    CHASSIS_STOP,
    CHASSIS_NORMAL,
    CHASSIS_COORD,
    CHASSIS_TRACKING,
} ChassisCtrlMode_TypeDef;

typedef struct
{
    ChassisCtrlMode_TypeDef mode_order;
    ChassisCtrlMode_TypeDef mode_run;
    float Gyro_YawAngle_zero;
    float Gyro_YawAngle_Calc;
    float Gyro_YawAngle_Chassis;
    float Gyro_YawAngle_Coord;
    ChassisHandle_TypeDef chassis_struct;
    struct {
        float goal_yaw, goal_x, goal_y;
        float soft_yaw, soft_x, soft_y;
        Location_Tracker_Typedef Yaw_Tracker_Struct;
        Location_Tracker_Typedef X_Tracker_Struct;
        Location_Tracker_Typedef Y_Tracker_Struct;
    } ChassisCoord_CtrlStruct;
    pid_t YawAngle_pid;
    pid_t LocationX_pid;
    pid_t LocationY_pid;
    volatile float *qGyro_YawAngle_New;
} RCCUStruct_TypeDef;
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern RCCUStruct_TypeDef rccu_struct;
/* Exported functions --------------------------------------------------------*/
void ChassisCoord_Set(float _x_diff, float _y_diff, float _yaw_diff);
void ChassisCoord_WaitStop(void);
void ServoSetPluseAndTime(uint8_t mode, uint16_t pwmval,uint16_t time);
void ChassisSpeed_Set(float _x_spd, float _y_spd);
int32_t Read_Position_x_mm(void);
int32_t Read_Position_y_mm(void);
void rccu_setmode_to_tracking( void );
void ChassisRELAX_set(void);
float Read_Position_yaw(void);
void ChassisModle_Set(int model);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __THREAD_RCCU_H_ */


