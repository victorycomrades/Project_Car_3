########################################
# CONSTANTS
########################################
# Motor
uint8 LEFT_FRONT_MOTOR  = 0
uint8 RIGHT_FRONT_MOTOR = 1
uint8 LEFT_BACK_MOTOR  = 2
uint8 RIGHT_BACK_MOTOR = 3

# Sensor
uint8 FRONT_SENSOR  = 0
uint8 RIGHT_SENSOR = 1
uint8 BACK_SENSOR  = 2
uint8 LEFT_SENSOR = 3

########################################
# Messages
########################################
Header  header
float32[4] sonar
uint8[4] illumination
int32[4] motor_encoder
uint8[7] button
float32  battery