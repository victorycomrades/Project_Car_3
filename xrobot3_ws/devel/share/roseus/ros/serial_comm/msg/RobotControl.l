;; Auto-generated. Do not edit!


(when (boundp 'serial_comm::RobotControl)
  (if (not (find-package "SERIAL_COMM"))
    (make-package "SERIAL_COMM"))
  (shadow 'RobotControl (find-package "SERIAL_COMM")))
(unless (find-package "SERIAL_COMM::ROBOTCONTROL")
  (make-package "SERIAL_COMM::ROBOTCONTROL"))

(in-package "ROS")
;;//! \htmlinclude RobotControl.msg.html


(defclass serial_comm::RobotControl
  :super ros::object
  :slots (_motor1_speed _motor2_speed _motor3_speed _motor4_speed _joint1_angle _joint2_angle _joint3_angle _wrist_angle _gripper_angle _rest_flag ))

(defmethod serial_comm::RobotControl
  (:init
   (&key
    ((:motor1_speed __motor1_speed) 0)
    ((:motor2_speed __motor2_speed) 0)
    ((:motor3_speed __motor3_speed) 0)
    ((:motor4_speed __motor4_speed) 0)
    ((:joint1_angle __joint1_angle) 0)
    ((:joint2_angle __joint2_angle) 0)
    ((:joint3_angle __joint3_angle) 0)
    ((:wrist_angle __wrist_angle) 0)
    ((:gripper_angle __gripper_angle) 0)
    ((:rest_flag __rest_flag) 0)
    )
   (send-super :init)
   (setq _motor1_speed (round __motor1_speed))
   (setq _motor2_speed (round __motor2_speed))
   (setq _motor3_speed (round __motor3_speed))
   (setq _motor4_speed (round __motor4_speed))
   (setq _joint1_angle (round __joint1_angle))
   (setq _joint2_angle (round __joint2_angle))
   (setq _joint3_angle (round __joint3_angle))
   (setq _wrist_angle (round __wrist_angle))
   (setq _gripper_angle (round __gripper_angle))
   (setq _rest_flag (round __rest_flag))
   self)
  (:motor1_speed
   (&optional __motor1_speed)
   (if __motor1_speed (setq _motor1_speed __motor1_speed)) _motor1_speed)
  (:motor2_speed
   (&optional __motor2_speed)
   (if __motor2_speed (setq _motor2_speed __motor2_speed)) _motor2_speed)
  (:motor3_speed
   (&optional __motor3_speed)
   (if __motor3_speed (setq _motor3_speed __motor3_speed)) _motor3_speed)
  (:motor4_speed
   (&optional __motor4_speed)
   (if __motor4_speed (setq _motor4_speed __motor4_speed)) _motor4_speed)
  (:joint1_angle
   (&optional __joint1_angle)
   (if __joint1_angle (setq _joint1_angle __joint1_angle)) _joint1_angle)
  (:joint2_angle
   (&optional __joint2_angle)
   (if __joint2_angle (setq _joint2_angle __joint2_angle)) _joint2_angle)
  (:joint3_angle
   (&optional __joint3_angle)
   (if __joint3_angle (setq _joint3_angle __joint3_angle)) _joint3_angle)
  (:wrist_angle
   (&optional __wrist_angle)
   (if __wrist_angle (setq _wrist_angle __wrist_angle)) _wrist_angle)
  (:gripper_angle
   (&optional __gripper_angle)
   (if __gripper_angle (setq _gripper_angle __gripper_angle)) _gripper_angle)
  (:rest_flag
   (&optional __rest_flag)
   (if __rest_flag (setq _rest_flag __rest_flag)) _rest_flag)
  (:serialization-length
   ()
   (+
    ;; int16 _motor1_speed
    2
    ;; int16 _motor2_speed
    2
    ;; int16 _motor3_speed
    2
    ;; int16 _motor4_speed
    2
    ;; int16 _joint1_angle
    2
    ;; int16 _joint2_angle
    2
    ;; int16 _joint3_angle
    2
    ;; int16 _wrist_angle
    2
    ;; int16 _gripper_angle
    2
    ;; uint8 _rest_flag
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int16 _motor1_speed
       (write-word _motor1_speed s)
     ;; int16 _motor2_speed
       (write-word _motor2_speed s)
     ;; int16 _motor3_speed
       (write-word _motor3_speed s)
     ;; int16 _motor4_speed
       (write-word _motor4_speed s)
     ;; int16 _joint1_angle
       (write-word _joint1_angle s)
     ;; int16 _joint2_angle
       (write-word _joint2_angle s)
     ;; int16 _joint3_angle
       (write-word _joint3_angle s)
     ;; int16 _wrist_angle
       (write-word _wrist_angle s)
     ;; int16 _gripper_angle
       (write-word _gripper_angle s)
     ;; uint8 _rest_flag
       (write-byte _rest_flag s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int16 _motor1_speed
     (setq _motor1_speed (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _motor2_speed
     (setq _motor2_speed (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _motor3_speed
     (setq _motor3_speed (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _motor4_speed
     (setq _motor4_speed (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _joint1_angle
     (setq _joint1_angle (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _joint2_angle
     (setq _joint2_angle (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _joint3_angle
     (setq _joint3_angle (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _wrist_angle
     (setq _wrist_angle (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _gripper_angle
     (setq _gripper_angle (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint8 _rest_flag
     (setq _rest_flag (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get serial_comm::RobotControl :md5sum-) "c444a667b2720c4168deb47dd4f537a3")
(setf (get serial_comm::RobotControl :datatype-) "serial_comm/RobotControl")
(setf (get serial_comm::RobotControl :definition-)
      "int16 motor1_speed
int16 motor2_speed
int16 motor3_speed
int16 motor4_speed
int16 joint1_angle
int16 joint2_angle
int16 joint3_angle
int16 wrist_angle
int16 gripper_angle
uint8 rest_flag
")



(provide :serial_comm/RobotControl "c444a667b2720c4168deb47dd4f537a3")


