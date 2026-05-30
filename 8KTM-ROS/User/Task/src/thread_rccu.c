/* Includes ------------------------------------------------------------------*/
#include "thread_comm.h"
#include "thread_rccu.h"
#include "bsp.h"
#include "data.h"
#include "thread_gui.h"
#include "chassis_function.h"
#include "pid.h"
#include "Location_Tracker.h"
#include "thread_user.h"
#include <stdio.h>
/* Private macros ------------------------------------------------------------*/
//锟斤拷锟斤拷频锟斤拷_hz
#define CONTROL_FREQ_HZ			  (100)
//YAW PID锟斤拷锟斤拷
#define De_YAW_KP                 5.0f
#define De_YAW_KI                 0.0f
#define De_YAW_KD                 5.0f
//X PID锟斤拷锟斤拷
#define De_LOCX_KP                2.8f
#define De_LOCX_KI                0.0f
#define De_LOCX_KD                5.0f
//Y PID锟斤拷锟斤拷
#define De_LOCY_KP                2.8f
#define De_LOCY_KI                0.0f
#define De_LOCY_KD                5.0f
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
float GyroYawAngleCalc = 0;
/* 锟斤拷锟斤拷锟竭程匡拷锟狡匡拷指锟斤拷 */
rt_thread_t thread_rccu = RT_NULL;
/* 锟斤拷锟教匡拷锟斤拷 */
RCCUStruct_TypeDef rccu_struct;
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  锟斤拷锟侥Ｊ斤拷锟斤拷锟?  * @param  _mode
  * @retval NULL
**/
static void rccu_setmode( RCCUStruct_TypeDef *rccu_handle, ChassisCtrlMode_TypeDef _mode )
{
	rccu_handle->mode_order = _mode;
}
static float Read_RealYawAngle( RCCUStruct_TypeDef* rccu_handle )
{
	return (*rccu_handle->qGyro_YawAngle_New);
}
static float Read_GyroYawAngleCalc( RCCUStruct_TypeDef* rccu_handle )
{//锟斤拷取锟斤拷锟斤拷锟斤拷值
	#ifdef HWT101_gyro
		float YawAngleCalc;
		YawAngleCalc = Read_RealYawAngle(rccu_handle) - rccu_handle->Gyro_YawAngle_zero;
		if(YawAngleCalc < 0)
			YawAngleCalc += 360;
		if(YawAngleCalc > 360)
			YawAngleCalc -= 360;
		GyroYawAngleCalc = YawAngleCalc;
		return YawAngleCalc;
	#else
		return zangle;//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷yaw锟斤拷
	#endif
}
static void rccu_chassisctrl( RCCUStruct_TypeDef* rccu_handle )
{//小锟斤拷锟斤拷锟教匡拷锟斤拷
	int16_t speed1buff;
    int16_t speed2buff;
    int16_t speed3buff;
    int16_t speed4buff;
	
	int32_t distance11buff;
	int32_t distance12buff;
	int32_t distance13buff;
	int32_t distance14buff;
	
	if(rccu_struct.chassis_struct.Chassis_CtrlFunc != 0)
		rccu_struct.chassis_struct.Chassis_CtrlFunc(&rccu_struct.chassis_struct);
    /**************************锟斤拷锟斤拷锟劫讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷*******************************/	
	speed1buff = rccu_handle->chassis_struct.wheel_rpm[0];
	speed2buff = rccu_handle->chassis_struct.wheel_rpm[1];
	speed3buff = rccu_handle->chassis_struct.wheel_rpm[2];
	speed4buff = rccu_handle->chassis_struct.wheel_rpm[3];
#if (DCMOTOR1_DIR == REV)
	 speed1buff = -speed1buff;
#endif
#if (DCMOTOR2_DIR == REV)
	 speed2buff = -speed2buff;
#endif
#if (DCMOTOR3_DIR == REV)
	 speed3buff = -speed3buff;
#endif
#if (DCMOTOR4_DIR == REV)
	 speed4buff = -speed4buff;
#endif
    #if 1
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt1_S,speed1buff);
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt2_S,speed2buff);
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt3_S,speed3buff);
		SLAVE_DCMotorMiniwatt_SpeedSet(&DCMotorMiniwatt4_S,speed4buff);
		#else
		SLAVE_DCMotorMiniwatt_Digital_SpeedSet(&DCMotorMiniwatt1_S,speed1buff);
		SLAVE_DCMotorMiniwatt_Digital_SpeedSet(&DCMotorMiniwatt2_S,speed2buff);
		SLAVE_DCMotorMiniwatt_Digital_SpeedSet(&DCMotorMiniwatt3_S,speed3buff);
		SLAVE_DCMotorMiniwatt_Digital_SpeedSet(&DCMotorMiniwatt4_S,speed4buff);
		#endif
	/**************************锟斤拷锟斤拷锟斤拷菁锟斤拷愕斤拷锟斤拷锟斤拷锟?******************************/
	rccu_handle->chassis_struct.yaw_gyro_angle = rccu_handle->Gyro_YawAngle_Calc;
	speed1buff = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt1_S);
	speed2buff = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt2_S);
	speed3buff = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt3_S);
	speed4buff = SLAVE_DCMotorMiniwatt_SpeedRead(&DCMotorMiniwatt4_S);
	distance11buff = SLAVE_DCMotorMiniwatt_PlaceRead(&DCMotorMiniwatt1_S);
	distance12buff = SLAVE_DCMotorMiniwatt_PlaceRead(&DCMotorMiniwatt2_S);
	distance13buff = SLAVE_DCMotorMiniwatt_PlaceRead(&DCMotorMiniwatt3_S);
	distance14buff = SLAVE_DCMotorMiniwatt_PlaceRead(&DCMotorMiniwatt4_S);
	#if (DCMOTOR1_DIR == REV)
	 speed1buff = -speed1buff;
	 distance11buff = -distance11buff;
	#endif
	#if (DCMOTOR2_DIR == REV)
		 speed2buff = -speed2buff;
		 distance12buff = -distance12buff;
	#endif
	#if (DCMOTOR3_DIR == REV)
		 speed3buff = -speed3buff;
		 distance13buff = -distance13buff;
	#endif
	#if (DCMOTOR4_DIR == REV)
		 speed4buff = -speed4buff;
		 distance14buff = -distance14buff;
	#endif
	rccu_handle->chassis_struct.Chassis_Calc_ForwardTransform( &rccu_handle->chassis_struct,
															   speed1buff,
															   speed2buff,
															   speed3buff,
															   speed4buff,
															   distance11buff,	
															   distance12buff,
															   distance13buff,
															   distance14buff );
}
static void rccu_chassisctrl_normalmode( RCCUStruct_TypeDef* rccu_handle )
{//锟斤拷锟斤拷锟斤拷锟角碉拷锟斤拷锟斤拷模式(锟睫硷拷锟斤拷锟斤拷锟?
	float YawAngle_Diff;

	YawAngle_Diff = rccu_handle->Gyro_YawAngle_Calc - rccu_handle->ChassisCoord_CtrlStruct.soft_yaw;
	while(YawAngle_Diff < -180)
		YawAngle_Diff += 360;
	while(YawAngle_Diff > 180)
		YawAngle_Diff -= 360;
	
	rccu_handle->chassis_struct.vw = rccu_handle->YawAngle_pid.f_pid_calc( &rccu_handle->YawAngle_pid,YawAngle_Diff,0 );
	rccu_handle->chassis_struct.gimbal_yaw_ecd_angle = 0;
	rccu_handle->chassis_struct.vx = sRemoteCtrl_Info.sPosition.vx;
	rccu_handle->chassis_struct.vy = sRemoteCtrl_Info.sPosition.vy;
}
static void rccu_chassisctrl_coordmode( RCCUStruct_TypeDef* rccu_handle )
{//锟斤拷锟斤拷锟斤拷锟角碉拷锟斤拷锟斤拷模式(锟斤拷锟斤拷锟斤拷锟?
	float YawAngle_Diff;

	YawAngle_Diff = rccu_handle->Gyro_YawAngle_Calc - rccu_handle->ChassisCoord_CtrlStruct.soft_yaw;
	while(YawAngle_Diff < -180)
		YawAngle_Diff += 360;
	while(YawAngle_Diff > 180)
		YawAngle_Diff -= 360;
	
	rccu_handle->chassis_struct.vw = rccu_handle->YawAngle_pid.f_pid_calc( &rccu_handle->YawAngle_pid,YawAngle_Diff,0 );
	rccu_handle->chassis_struct.vx = rccu_handle->LocationX_pid.f_pid_calc( &rccu_handle->LocationX_pid, \
	                                                                        rccu_handle->chassis_struct.position.position_x_mm, \
	                                                                        rccu_handle->ChassisCoord_CtrlStruct.soft_x );
	
	rccu_handle->chassis_struct.vy = rccu_handle->LocationY_pid.f_pid_calc( &rccu_handle->LocationY_pid, \
	                                                                        rccu_handle->chassis_struct.position.position_y_mm, \
	                                                                        rccu_handle->ChassisCoord_CtrlStruct.soft_y );
	
	/************************************锟斤拷锟斤拷锟斤拷锟?*******************************************/
	YawAngle_Diff = rccu_handle->Gyro_YawAngle_Coord - rccu_handle->Gyro_YawAngle_Calc;  
	if(YawAngle_Diff < 0)
		YawAngle_Diff += 360;
	if(YawAngle_Diff > 360)
		YawAngle_Diff -= 360;
	rccu_handle->chassis_struct.gimbal_yaw_ecd_angle = YawAngle_Diff;
}
static void rccu_chassisctrl_trackingmode( RCCUStruct_TypeDef* rccu_handle )
{
	rccu_handle->chassis_struct.gimbal_yaw_ecd_angle = 0;
	rccu_handle->chassis_struct.vx = rccu_handle->ChassisCoord_CtrlStruct.soft_x;
    rccu_handle->chassis_struct.vy = rccu_handle->ChassisCoord_CtrlStruct.soft_y;
	rccu_handle->chassis_struct.vw = rccu_handle->ChassisCoord_CtrlStruct.soft_yaw;
}
/**
  * @brief  锟斤拷锟斤拷锟斤拷瞥锟绞硷拷锟?  * @param  NULL
  * @retval NULL
**/
static void rccu_init( RCCUStruct_TypeDef* rccu_handle )
{
	memset(rccu_handle, 0, sizeof(RCCUStruct_TypeDef));
	rccu_setmode( rccu_handle, CHASSIS_RELAX );
//#######################################################################################################
//实时锟斤拷取锟斤拷锟斤拷锟角角讹拷
	#ifdef HWT101_gyro
		rccu_handle->qGyro_YawAngle_New = &GyroData_Struct.Yaw;
	#else 
		rccu_handle->qGyro_YawAngle_New = &zangle;
	#endif
//#######################################################################################################
	Chassis_Init( &rccu_handle->chassis_struct,
	              FOUR_DRIVE_McNamara,
								RC_CHASSIS_MAX_SPEED_X,   //锟斤拷锟絏锟斤拷锟劫讹拷
								RC_CHASSIS_MAX_SPEED_Y,   //锟斤拷锟結锟斤拷锟劫讹拷
								RC_CHASSIS_MAX_SPEED_R, MAX_WHEEL_RPM );
	PID_struct_init(&rccu_handle->YawAngle_pid, POSITION_PID, RC_CHASSIS_MAX_SPEED_R, 50.0f,De_YAW_KP, De_YAW_KI, De_YAW_KD);
	rccu_handle->YawAngle_pid.output_deadband = 0;
	//X PID
	PID_struct_init(&rccu_handle->LocationX_pid, POSITION_PID, RC_CHASSIS_MAX_SPEED_X, 50.0f,De_LOCX_KP, De_LOCX_KI, De_LOCX_KD);
	rccu_handle->LocationX_pid.output_deadband = 0;
	//Y PID
	PID_struct_init(&rccu_handle->LocationY_pid, POSITION_PID, RC_CHASSIS_MAX_SPEED_Y, 50.0f,De_LOCY_KP, De_LOCY_KI, De_LOCY_KD);
	rccu_handle->LocationY_pid.output_deadband = 0;
	//YAW 锟斤拷锟斤拷锟斤拷
	Location_Tracker_Init( &rccu_handle->ChassisCoord_CtrlStruct.Yaw_Tracker_Struct,
													CONTROL_FREQ_HZ,//CtrlFreqHZ
													RC_CHASSIS_MAX_SPEED_R,//max_speed
													RC_CHASSIS_MAX_SPEED_R*2,//up_acc	默锟斤拷*2/3
													RC_CHASSIS_MAX_SPEED_R*2,//down_acc 默锟斤拷*2/3
													RC_CHASSIS_MAX_SPEED_R );//speed_locking_stop
	//X 锟斤拷锟斤拷锟斤拷
	Location_Tracker_Init( &rccu_handle->ChassisCoord_CtrlStruct.X_Tracker_Struct,
													CONTROL_FREQ_HZ,//CtrlFreqHZ
													RC_CHASSIS_MAX_SPEED_X,//max_speed
													RC_CHASSIS_MAX_SPEED_X/1,//up_acc	默锟斤拷/4
													RC_CHASSIS_MAX_SPEED_X/1,//down_acc 默锟斤拷/4
													RC_CHASSIS_MAX_SPEED_X );//speed_locking_stop
	//Y 锟斤拷锟斤拷锟斤拷
	Location_Tracker_Init( &rccu_handle->ChassisCoord_CtrlStruct.Y_Tracker_Struct,
													CONTROL_FREQ_HZ,
													RC_CHASSIS_MAX_SPEED_Y,//max_speed
													RC_CHASSIS_MAX_SPEED_Y/1,//up_acc	默锟斤拷/4
													RC_CHASSIS_MAX_SPEED_Y/1,//down_acc 默锟斤拷/4
													RC_CHASSIS_MAX_SPEED_Y );//speed_locking_stop
	//循锟斤拷锟斤拷锟斤拷锟斤拷				   
	LineTracker_Init( 1, 
	                  CONTROL_FREQ_HZ, 
	                  &Tracking_Device1.Tracking_UploadData.DATE.SignalData,   //前
                      &Tracking_Device2.Tracking_UploadData.DATE.SignalData,   //锟斤拷
					  &Tracking_Device3.Tracking_UploadData.DATE.SignalData,   //锟斤拷
					  &Tracking_Device4.Tracking_UploadData.DATE.SignalData ); //锟斤拷
}
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void ChassisModle_Set(int model)
{
		switch(model)
	{
		case 1:
		rccu_struct.mode_order = CHASSIS_RELAX;//锟斤拷锟斤拷锟斤拷
		break;
		case 2:
		rccu_struct.mode_order = CHASSIS_COORD;//x锟斤拷y锟斤拷yaw锟斤拷锟皆斤拷锟斤拷
		break;
		case 3:
		rccu_struct.mode_order = CHASSIS_NORMAL;//yaw锟斤拷锟皆斤拷锟斤拷
		break;
	}
}
void ChassisCoord_Set(float _x_diff, float _y_diff, float _yaw_diff)
{
	if( rccu_struct.mode_order != CHASSIS_COORD )
	{
		rccu_struct.mode_order = CHASSIS_COORD;
		My_mDelay(100);
	}
	#ifdef EncodingWheel//X Y锟斤拷锟斤拷位锟斤拷锟斤拷锟斤拷	,Z锟斤拷锟轿伙拷锟斤拷锟斤拷锟?			rccu_struct.ChassisCoord_CtrlStruct.goal_y = _y_diff;
			rccu_struct.ChassisCoord_CtrlStruct.goal_x = _x_diff;
			rccu_struct.ChassisCoord_CtrlStruct.goal_yaw += _yaw_diff;
  #else
			rccu_struct.ChassisCoord_CtrlStruct.goal_y += _y_diff;
			rccu_struct.ChassisCoord_CtrlStruct.goal_x += _x_diff;
			rccu_struct.ChassisCoord_CtrlStruct.goal_yaw += _yaw_diff;
	#endif
#if 0	
	while( ( rccu_struct.ChassisCoord_CtrlStruct.goal_yaw < 0 ) || \
		   ( rccu_struct.ChassisCoord_CtrlStruct.goal_yaw > 360 ) )
	{
		if(rccu_struct.ChassisCoord_CtrlStruct.goal_yaw < 0)
			rccu_struct.ChassisCoord_CtrlStruct.goal_yaw += 360;
		if(rccu_struct.ChassisCoord_CtrlStruct.goal_yaw >= 360)
			rccu_struct.ChassisCoord_CtrlStruct.goal_yaw -= 360;
	}
#endif
}
//void ChassisCoord_WaitStop(void)
//{
//	while( ( rccu_struct.ChassisCoord_CtrlStruct.soft_y != rccu_struct.ChassisCoord_CtrlStruct.goal_y ) || \
//		   ( ABS(rccu_struct.LocationY_pid.err[NOW]) > 10 ) || \
//		   ( rccu_struct.ChassisCoord_CtrlStruct.soft_x != rccu_struct.ChassisCoord_CtrlStruct.goal_x ) || \
//		   ( ABS(rccu_struct.LocationX_pid.err[NOW]) > 10 ) || \
//		   ( ABS(rccu_struct.ChassisCoord_CtrlStruct.soft_yaw - rccu_struct.ChassisCoord_CtrlStruct.goal_yaw) > 0.5f) || \
//		   ( ABS(rccu_struct.YawAngle_pid.err[NOW]) > 0.2f ) )
//	{
//		My_mDelay(10);
//	}
//}

void ChassisCoord_WaitStop(void)
{
	while( ( rccu_struct.ChassisCoord_CtrlStruct.soft_y != rccu_struct.ChassisCoord_CtrlStruct.goal_y ) || \
		   ( ABS(rccu_struct.LocationY_pid.err[NOW]) > 150 ) || \
		   ( rccu_struct.ChassisCoord_CtrlStruct.soft_x != rccu_struct.ChassisCoord_CtrlStruct.goal_x ) || \
		   ( ABS(rccu_struct.LocationX_pid.err[NOW]) > 150 ) || \
		   ( ABS(rccu_struct.ChassisCoord_CtrlStruct.soft_yaw - rccu_struct.ChassisCoord_CtrlStruct.goal_yaw) > 0.5f) || \
		   ( ABS(rccu_struct.YawAngle_pid.err[NOW]) > 8.0f ) )
	{
		My_mDelay(10);
	}
}
void ChassisSpeed_Set(float _x_spd, float _y_spd)
{
	if( rccu_struct.mode_order != CHASSIS_NORMAL )
	{
		rccu_struct.mode_order = CHASSIS_NORMAL;
		My_mDelay(100);
	}
	sRemoteCtrl_Info.sPosition.vx = _x_spd;
	sRemoteCtrl_Info.sPosition.vy = _y_spd;
}
void ChassisRELAX_set(void)
{
	if( rccu_struct.mode_order != CHASSIS_RELAX )
	{
		rccu_struct.mode_order = CHASSIS_RELAX;
		My_mDelay(100);
	}
}
float Read_Position_yaw(void)
{
	#ifdef Disable_gyro_WhenArmMOVE
			return rccu_struct.ChassisCoord_CtrlStruct.goal_yaw;
	#elif defined(HWT101_gyro)
			return GyroYawAngleCalc;
	#elif defined(EncodingWheel)
			return zangle;
	#else
			return 0.0f;//锟斤拷锟较讹拷没锟叫就凤拷锟斤拷
	#endif
}
void rccu_setmode_to_tracking( void )
{
	rccu_struct.mode_order = CHASSIS_TRACKING;
}
int32_t Read_Position_x_mm(void)
{
	return rccu_struct.chassis_struct.position.position_x_mm;
}
int32_t Read_Position_y_mm(void)
{
	return rccu_struct.chassis_struct.position.position_y_mm;
}
void rccu_task(void *pvParameters)
{
	//锟斤拷时锟饺达拷锟斤拷锟斤拷锟饺讹拷
	My_mDelay(1000);
	//锟斤拷取锟斤拷前锟斤拷锟斤拷
	rccu_struct.Gyro_YawAngle_zero = Read_RealYawAngle(&rccu_struct);
	rccu_struct.Gyro_YawAngle_Calc = Read_GyroYawAngleCalc(&rccu_struct);
			/* ------ debug: print encoding wheel data every 1s ------ */
	rccu_struct.Gyro_YawAngle_Chassis = rccu_struct.Gyro_YawAngle_Calc;
	rccu_struct.Gyro_YawAngle_Coord = rccu_struct.Gyro_YawAngle_Calc;
	
	rccu_setmode( &rccu_struct, CHASSIS_RELAX );//锟斤拷始模式
	Task_User_create();
	while(1)
	{
		/************************ 锟剿讹拷锟斤拷锟斤拷 ************************************/
		/************************ 锟剿讹拷锟斤拷锟斤拷 ************************************/
		switch ( rccu_struct.mode_run )
		{
			case CHASSIS_RELAX:
			case CHASSIS_STOP:
			{//停止
				rccu_struct.chassis_struct.vx = 0;
				rccu_struct.chassis_struct.vy = 0;
				rccu_struct.chassis_struct.vw = 0;
			}break;
			case CHASSIS_NORMAL:
			{//锟斤拷锟斤拷模式
				rccu_chassisctrl_normalmode(&rccu_struct);
			}break;
			case CHASSIS_COORD:
			{//锟斤拷锟斤拷模式
				rccu_chassisctrl_coordmode(&rccu_struct);
			}break;
			case CHASSIS_TRACKING:
			{//循锟斤拷模式
				rccu_chassisctrl_trackingmode(&rccu_struct);
			}break;
			default:
				break;
		}
		rccu_chassisctrl( &rccu_struct );
		/************************ 模式锟斤拷锟?************************************/
		/************************ 模式锟斤拷锟?************************************/
		if(rccu_struct.mode_run != rccu_struct.mode_order)
	    {//锟斤拷锟?			rccu_struct.mode_run = rccu_struct.mode_order;
			switch (rccu_struct.mode_run)
			{
				case CHASSIS_RELAX:
				case CHASSIS_STOP:
				case CHASSIS_NORMAL:  //遥锟斤拷
					rccu_struct.Gyro_YawAngle_Chassis = rccu_struct.Gyro_YawAngle_Calc;
					break;
				case CHASSIS_COORD:   //锟斤拷锟斤拷模式
						rccu_struct.ChassisCoord_CtrlStruct.soft_yaw = rccu_struct.Gyro_YawAngle_Calc;
				    rccu_struct.ChassisCoord_CtrlStruct.goal_yaw = rccu_struct.ChassisCoord_CtrlStruct.soft_yaw;
				    Location_Tracker_NewTask( &rccu_struct.ChassisCoord_CtrlStruct.Yaw_Tracker_Struct, \
	                                          rccu_struct.ChassisCoord_CtrlStruct.soft_yaw, \
                                              0 );
				    rccu_struct.ChassisCoord_CtrlStruct.soft_x = rccu_struct.chassis_struct.position.position_x_mm;
				    rccu_struct.ChassisCoord_CtrlStruct.goal_x = rccu_struct.ChassisCoord_CtrlStruct.soft_x;
				    Location_Tracker_NewTask( &rccu_struct.ChassisCoord_CtrlStruct.X_Tracker_Struct, \
	                                          rccu_struct.ChassisCoord_CtrlStruct.soft_x, \
                                              0 );
				    rccu_struct.ChassisCoord_CtrlStruct.soft_y = rccu_struct.chassis_struct.position.position_y_mm;
				    rccu_struct.ChassisCoord_CtrlStruct.goal_y = rccu_struct.ChassisCoord_CtrlStruct.soft_y;
				    Location_Tracker_NewTask( &rccu_struct.ChassisCoord_CtrlStruct.Y_Tracker_Struct, \
	                                          rccu_struct.ChassisCoord_CtrlStruct.soft_y, \
                                              0 );
					break;
				default:
					break;
			}
		}
		/************************ 锟斤拷目锟斤拷锟斤拷取 **********************************/
		/************************ 锟斤拷目锟斤拷锟斤拷取 **********************************/
		switch (rccu_struct.mode_run)
		{
			case CHASSIS_COORD://锟斤拷锟斤拷模式
				Location_Tracker_Capture_Goal( &rccu_struct.ChassisCoord_CtrlStruct.Yaw_Tracker_Struct, \
			                                   rccu_struct.ChassisCoord_CtrlStruct.goal_yaw );
	            if(ABS(rccu_struct.ChassisCoord_CtrlStruct.soft_yaw-rccu_struct.ChassisCoord_CtrlStruct.goal_yaw) < 1.0f)
					rccu_struct.ChassisCoord_CtrlStruct.soft_yaw = rccu_struct.ChassisCoord_CtrlStruct.goal_yaw;
				else
			        rccu_struct.ChassisCoord_CtrlStruct.soft_yaw = rccu_struct.ChassisCoord_CtrlStruct.Yaw_Tracker_Struct.go_location;
			    Location_Tracker_Capture_Goal( &rccu_struct.ChassisCoord_CtrlStruct.X_Tracker_Struct, \
			                                   rccu_struct.ChassisCoord_CtrlStruct.goal_x );
	            rccu_struct.ChassisCoord_CtrlStruct.soft_x = rccu_struct.ChassisCoord_CtrlStruct.X_Tracker_Struct.go_location;
			
			    Location_Tracker_Capture_Goal( &rccu_struct.ChassisCoord_CtrlStruct.Y_Tracker_Struct, \
			                                   rccu_struct.ChassisCoord_CtrlStruct.goal_y );
	            rccu_struct.ChassisCoord_CtrlStruct.soft_y = rccu_struct.ChassisCoord_CtrlStruct.Y_Tracker_Struct.go_location;
			    break;
			case CHASSIS_TRACKING:
				LineTracker_Scan();
			    rccu_struct.ChassisCoord_CtrlStruct.soft_yaw = LineTracker_Struct.yaw_PracticalOut;
			    rccu_struct.ChassisCoord_CtrlStruct.soft_x = LineTracker_Struct.x_axis_PracticalOut;
			    rccu_struct.ChassisCoord_CtrlStruct.soft_y = LineTracker_Struct.y_axis_PracticalOut;
			default:
				break;
		}
		My_mDelay( 10 );
	}
}
int Task_RCCU_create(void)
{
	rccu_init( &rccu_struct );
	
    thread_rccu = rt_thread_create("rccu",            /* 锟竭筹拷锟斤拷锟斤拷 */
								   rccu_task,         /* 锟竭筹拷锟斤拷诤锟斤拷锟?*/
								   RT_NULL,           /* 锟竭筹拷锟斤拷诤锟斤拷锟斤拷锟斤拷锟?*/
								   1024,              /* 锟竭筹拷栈锟斤拷小 */
								   2,                 /* 锟竭程碉拷锟斤拷锟饺硷拷 */
								   20);               /* 锟竭筹拷时锟斤拷片 */
	if(thread_rccu != RT_NULL)
	{
		rt_thread_startup(thread_rccu);
		rt_kprintf("thread_rccu startup!\n");
	}
	return 0;
}
//INIT_APP_EXPORT(Task_RCCU_create);
