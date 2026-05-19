;; Auto-generated. Do not edit!


(when (boundp 'xrobot_msgs::ParamConfig)
  (if (not (find-package "XROBOT_MSGS"))
    (make-package "XROBOT_MSGS"))
  (shadow 'ParamConfig (find-package "XROBOT_MSGS")))
(unless (find-package "XROBOT_MSGS::PARAMCONFIG")
  (make-package "XROBOT_MSGS::PARAMCONFIG"))

(in-package "ROS")
;;//! \htmlinclude ParamConfig.msg.html


(intern "*DIFFERENTIAL_DRIVE*" (find-package "XROBOT_MSGS::PARAMCONFIG"))
(shadow '*DIFFERENTIAL_DRIVE* (find-package "XROBOT_MSGS::PARAMCONFIG"))
(defconstant xrobot_msgs::ParamConfig::*DIFFERENTIAL_DRIVE* 0)
(intern "*ACKERMANN*" (find-package "XROBOT_MSGS::PARAMCONFIG"))
(shadow '*ACKERMANN* (find-package "XROBOT_MSGS::PARAMCONFIG"))
(defconstant xrobot_msgs::ParamConfig::*ACKERMANN* 1)
(intern "*MECANUM*" (find-package "XROBOT_MSGS::PARAMCONFIG"))
(shadow '*MECANUM* (find-package "XROBOT_MSGS::PARAMCONFIG"))
(defconstant xrobot_msgs::ParamConfig::*MECANUM* 2)

(defun xrobot_msgs::ParamConfig-to-symbol (const)
  (cond
        ((= const 0) 'xrobot_msgs::ParamConfig::*DIFFERENTIAL_DRIVE*)
        ((= const 1) 'xrobot_msgs::ParamConfig::*ACKERMANN*)
        ((= const 2) 'xrobot_msgs::ParamConfig::*MECANUM*)
        (t nil)))

(defclass xrobot_msgs::ParamConfig
  :super ros::object
  :slots (_chassis_type _x_speed_scale _y_speed_scale _w_speed_scale ))

(defmethod xrobot_msgs::ParamConfig
  (:init
   (&key
    ((:chassis_type __chassis_type) 0)
    ((:x_speed_scale __x_speed_scale) 0.0)
    ((:y_speed_scale __y_speed_scale) 0.0)
    ((:w_speed_scale __w_speed_scale) 0.0)
    )
   (send-super :init)
   (setq _chassis_type (round __chassis_type))
   (setq _x_speed_scale (float __x_speed_scale))
   (setq _y_speed_scale (float __y_speed_scale))
   (setq _w_speed_scale (float __w_speed_scale))
   self)
  (:chassis_type
   (&optional __chassis_type)
   (if __chassis_type (setq _chassis_type __chassis_type)) _chassis_type)
  (:x_speed_scale
   (&optional __x_speed_scale)
   (if __x_speed_scale (setq _x_speed_scale __x_speed_scale)) _x_speed_scale)
  (:y_speed_scale
   (&optional __y_speed_scale)
   (if __y_speed_scale (setq _y_speed_scale __y_speed_scale)) _y_speed_scale)
  (:w_speed_scale
   (&optional __w_speed_scale)
   (if __w_speed_scale (setq _w_speed_scale __w_speed_scale)) _w_speed_scale)
  (:serialization-length
   ()
   (+
    ;; uint8 _chassis_type
    1
    ;; float32 _x_speed_scale
    4
    ;; float32 _y_speed_scale
    4
    ;; float32 _w_speed_scale
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _chassis_type
       (write-byte _chassis_type s)
     ;; float32 _x_speed_scale
       (sys::poke _x_speed_scale (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y_speed_scale
       (sys::poke _y_speed_scale (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _w_speed_scale
       (sys::poke _w_speed_scale (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _chassis_type
     (setq _chassis_type (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _x_speed_scale
     (setq _x_speed_scale (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y_speed_scale
     (setq _y_speed_scale (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _w_speed_scale
     (setq _w_speed_scale (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get xrobot_msgs::ParamConfig :md5sum-) "f701675f55a47d8728e25b639a888073")
(setf (get xrobot_msgs::ParamConfig :datatype-) "xrobot_msgs/ParamConfig")
(setf (get xrobot_msgs::ParamConfig :definition-)
      "########################################
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
")



(provide :xrobot_msgs/ParamConfig "f701675f55a47d8728e25b639a888073")


