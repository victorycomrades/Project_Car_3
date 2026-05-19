########################################
# CONSTANTS
########################################
# Chassis Type
uint8 DIFFERENTIAL_DRIVE  = 0
uint8 ACKERMANN  = 1
uint8 MECANUM  = 2

# Arm Type

########################################
# Messages
########################################
uint8 chassis_type      # 底盘类型
float32 x_speed_scale     # 遥控的速度比例
float32 y_speed_scale 
float32 w_speed_scale