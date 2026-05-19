;; Auto-generated. Do not edit!


(when (boundp 'xrobot_msgs::SensorState)
  (if (not (find-package "XROBOT_MSGS"))
    (make-package "XROBOT_MSGS"))
  (shadow 'SensorState (find-package "XROBOT_MSGS")))
(unless (find-package "XROBOT_MSGS::SENSORSTATE")
  (make-package "XROBOT_MSGS::SENSORSTATE"))

(in-package "ROS")
;;//! \htmlinclude SensorState.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(intern "*LEFT_FRONT_MOTOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*LEFT_FRONT_MOTOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*LEFT_FRONT_MOTOR* 0)
(intern "*RIGHT_FRONT_MOTOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*RIGHT_FRONT_MOTOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*RIGHT_FRONT_MOTOR* 1)
(intern "*LEFT_BACK_MOTOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*LEFT_BACK_MOTOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*LEFT_BACK_MOTOR* 2)
(intern "*RIGHT_BACK_MOTOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*RIGHT_BACK_MOTOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*RIGHT_BACK_MOTOR* 3)
(intern "*FRONT_SENSOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*FRONT_SENSOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*FRONT_SENSOR* 0)
(intern "*RIGHT_SENSOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*RIGHT_SENSOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*RIGHT_SENSOR* 1)
(intern "*BACK_SENSOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*BACK_SENSOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*BACK_SENSOR* 2)
(intern "*LEFT_SENSOR*" (find-package "XROBOT_MSGS::SENSORSTATE"))
(shadow '*LEFT_SENSOR* (find-package "XROBOT_MSGS::SENSORSTATE"))
(defconstant xrobot_msgs::SensorState::*LEFT_SENSOR* 3)

(defun xrobot_msgs::SensorState-to-symbol (const)
  (cond
        ((= const 0) 'xrobot_msgs::SensorState::*LEFT_FRONT_MOTOR*)
        ((= const 1) 'xrobot_msgs::SensorState::*RIGHT_FRONT_MOTOR*)
        ((= const 2) 'xrobot_msgs::SensorState::*LEFT_BACK_MOTOR*)
        ((= const 3) 'xrobot_msgs::SensorState::*RIGHT_BACK_MOTOR*)
        ((= const 0) 'xrobot_msgs::SensorState::*FRONT_SENSOR*)
        ((= const 1) 'xrobot_msgs::SensorState::*RIGHT_SENSOR*)
        ((= const 2) 'xrobot_msgs::SensorState::*BACK_SENSOR*)
        ((= const 3) 'xrobot_msgs::SensorState::*LEFT_SENSOR*)
        (t nil)))

(defclass xrobot_msgs::SensorState
  :super ros::object
  :slots (_header _sonar _illumination _motor_encoder _button _battery ))

(defmethod xrobot_msgs::SensorState
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:sonar __sonar) (make-array 4 :initial-element 0.0 :element-type :float))
    ((:illumination __illumination) (make-array 4 :initial-element 0 :element-type :char))
    ((:motor_encoder __motor_encoder) (make-array 4 :initial-element 0 :element-type :integer))
    ((:button __button) (make-array 7 :initial-element 0 :element-type :char))
    ((:battery __battery) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _sonar __sonar)
   (setq _illumination __illumination)
   (setq _motor_encoder __motor_encoder)
   (setq _button __button)
   (setq _battery (float __battery))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:sonar
   (&optional __sonar)
   (if __sonar (setq _sonar __sonar)) _sonar)
  (:illumination
   (&optional __illumination)
   (if __illumination (setq _illumination __illumination)) _illumination)
  (:motor_encoder
   (&optional __motor_encoder)
   (if __motor_encoder (setq _motor_encoder __motor_encoder)) _motor_encoder)
  (:button
   (&optional __button)
   (if __button (setq _button __button)) _button)
  (:battery
   (&optional __battery)
   (if __battery (setq _battery __battery)) _battery)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float32[4] _sonar
    (* 4    4)
    ;; uint8[4] _illumination
    (* 1    4)
    ;; int32[4] _motor_encoder
    (* 4    4)
    ;; uint8[7] _button
    (* 1    7)
    ;; float32 _battery
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; float32[4] _sonar
     (dotimes (i 4)
       (sys::poke (elt _sonar i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; uint8[4] _illumination
     (princ _illumination s)
     ;; int32[4] _motor_encoder
     (dotimes (i 4)
       (write-long (elt _motor_encoder i) s)
       )
     ;; uint8[7] _button
     (princ _button s)
     ;; float32 _battery
       (sys::poke _battery (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; float32[4] _sonar
   (dotimes (i (length _sonar))
     (setf (elt _sonar i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     )
   ;; uint8[4] _illumination
   (setq _illumination (make-array 4 :element-type :char))
   (replace _illumination buf :start2 ptr-) (incf ptr- 4)
   ;; int32[4] _motor_encoder
   (dotimes (i (length _motor_encoder))
     (setf (elt _motor_encoder i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     )
   ;; uint8[7] _button
   (setq _button (make-array 7 :element-type :char))
   (replace _button buf :start2 ptr-) (incf ptr- 7)
   ;; float32 _battery
     (setq _battery (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get xrobot_msgs::SensorState :md5sum-) "229634ca6575e167dd4ffa0fff8b30f5")
(setf (get xrobot_msgs::SensorState :datatype-) "xrobot_msgs/SensorState")
(setf (get xrobot_msgs::SensorState :definition-)
      "########################################
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
================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

")



(provide :xrobot_msgs/SensorState "229634ca6575e167dd4ffa0fff8b30f5")


