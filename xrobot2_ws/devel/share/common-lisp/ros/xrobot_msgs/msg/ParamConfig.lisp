; Auto-generated. Do not edit!


(cl:in-package xrobot_msgs-msg)


;//! \htmlinclude ParamConfig.msg.html

(cl:defclass <ParamConfig> (roslisp-msg-protocol:ros-message)
  ((chassis_type
    :reader chassis_type
    :initarg :chassis_type
    :type cl:fixnum
    :initform 0)
   (x_speed_scale
    :reader x_speed_scale
    :initarg :x_speed_scale
    :type cl:float
    :initform 0.0)
   (y_speed_scale
    :reader y_speed_scale
    :initarg :y_speed_scale
    :type cl:float
    :initform 0.0)
   (w_speed_scale
    :reader w_speed_scale
    :initarg :w_speed_scale
    :type cl:float
    :initform 0.0))
)

(cl:defclass ParamConfig (<ParamConfig>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ParamConfig>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ParamConfig)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name xrobot_msgs-msg:<ParamConfig> is deprecated: use xrobot_msgs-msg:ParamConfig instead.")))

(cl:ensure-generic-function 'chassis_type-val :lambda-list '(m))
(cl:defmethod chassis_type-val ((m <ParamConfig>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:chassis_type-val is deprecated.  Use xrobot_msgs-msg:chassis_type instead.")
  (chassis_type m))

(cl:ensure-generic-function 'x_speed_scale-val :lambda-list '(m))
(cl:defmethod x_speed_scale-val ((m <ParamConfig>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:x_speed_scale-val is deprecated.  Use xrobot_msgs-msg:x_speed_scale instead.")
  (x_speed_scale m))

(cl:ensure-generic-function 'y_speed_scale-val :lambda-list '(m))
(cl:defmethod y_speed_scale-val ((m <ParamConfig>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:y_speed_scale-val is deprecated.  Use xrobot_msgs-msg:y_speed_scale instead.")
  (y_speed_scale m))

(cl:ensure-generic-function 'w_speed_scale-val :lambda-list '(m))
(cl:defmethod w_speed_scale-val ((m <ParamConfig>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:w_speed_scale-val is deprecated.  Use xrobot_msgs-msg:w_speed_scale instead.")
  (w_speed_scale m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<ParamConfig>)))
    "Constants for message type '<ParamConfig>"
  '((:DIFFERENTIAL_DRIVE . 0)
    (:ACKERMANN . 1)
    (:MECANUM . 2))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'ParamConfig)))
    "Constants for message type 'ParamConfig"
  '((:DIFFERENTIAL_DRIVE . 0)
    (:ACKERMANN . 1)
    (:MECANUM . 2))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ParamConfig>) ostream)
  "Serializes a message object of type '<ParamConfig>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'chassis_type)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'x_speed_scale))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'y_speed_scale))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'w_speed_scale))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ParamConfig>) istream)
  "Deserializes a message object of type '<ParamConfig>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'chassis_type)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'x_speed_scale) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'y_speed_scale) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'w_speed_scale) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ParamConfig>)))
  "Returns string type for a message object of type '<ParamConfig>"
  "xrobot_msgs/ParamConfig")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ParamConfig)))
  "Returns string type for a message object of type 'ParamConfig"
  "xrobot_msgs/ParamConfig")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ParamConfig>)))
  "Returns md5sum for a message object of type '<ParamConfig>"
  "f701675f55a47d8728e25b639a888073")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ParamConfig)))
  "Returns md5sum for a message object of type 'ParamConfig"
  "f701675f55a47d8728e25b639a888073")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ParamConfig>)))
  "Returns full string definition for message of type '<ParamConfig>"
  (cl:format cl:nil "########################################~%# CONSTANTS~%########################################~%# Chassis Type~%uint8 DIFFERENTIAL_DRIVE  = 0~%uint8 ACKERMANN  = 1~%uint8 MECANUM  = 2~%~%# Arm Type~%~%########################################~%# Messages~%########################################~%uint8 chassis_type      # 底盘类型~%float32 x_speed_scale     # 遥控的速度比例~%float32 y_speed_scale ~%float32 w_speed_scale~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ParamConfig)))
  "Returns full string definition for message of type 'ParamConfig"
  (cl:format cl:nil "########################################~%# CONSTANTS~%########################################~%# Chassis Type~%uint8 DIFFERENTIAL_DRIVE  = 0~%uint8 ACKERMANN  = 1~%uint8 MECANUM  = 2~%~%# Arm Type~%~%########################################~%# Messages~%########################################~%uint8 chassis_type      # 底盘类型~%float32 x_speed_scale     # 遥控的速度比例~%float32 y_speed_scale ~%float32 w_speed_scale~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ParamConfig>))
  (cl:+ 0
     1
     4
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ParamConfig>))
  "Converts a ROS message object to a list"
  (cl:list 'ParamConfig
    (cl:cons ':chassis_type (chassis_type msg))
    (cl:cons ':x_speed_scale (x_speed_scale msg))
    (cl:cons ':y_speed_scale (y_speed_scale msg))
    (cl:cons ':w_speed_scale (w_speed_scale msg))
))
