from jetarm import arm_v1servo
import serial
from time import *

ser = serial.Serial(port="/dev/XCOM1", baudrate=1000000)

arm = arm_v1servo.Arm_v1servo(ser, lower_arm_servo_reset_angle=0)

arm.angle_control(0,-5,-45,90,50,100)#star
#sleep(3)
#arm.angle_control(0,54,-55,90,50,100)
#sleep(10)
#arm.angle_control(0, 54, -30, 24, 50, 100)#grip
#sleep(5)

sleep(5)
arm.angle_control(0,-5,-45,24,50,100)#star

arm.angle_control(0,54,-55,24,50,100)#place
sleep(10)
arm.angle_control(0,14,-55,90,50,100)
sleep(10)
arm.angle_control(0,-5,-45,90,50,100)#reset

sleep(5)
arm.angle_control(0,-5,-45,24,50,100)#star

arm.angle_control(18,54,-55,24,50,100)
sleep(10)
arm.angle_control(18,14,-55,90,50,100)
sleep(10)
arm.angle_control(0,-5,-45,90,50,100)#reset
