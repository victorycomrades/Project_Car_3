from jetarm import  arm_v1servo
import serial
from time import *

ser = serial.Serial(port="/dev/XCOM1", baudrate=1000000)

arm = arm_v1servo.Arm_v1servo(ser, lower_arm_servo_reset_angle=0)


arm.angle_control(20, 0, 0, 0, 50, 20, 10, 400)
sleep(10)
arm.angle_control(3, 27, -45, 60, 0, 20, 10, 100)
sleep(5)
arm.angle_control(3, 27, -45, 60, -50, 20, 10, 100)
sleep(5)
 


