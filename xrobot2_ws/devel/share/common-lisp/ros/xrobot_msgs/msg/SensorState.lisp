; Auto-generated. Do not edit!


(cl:in-package xrobot_msgs-msg)


;//! \htmlinclude SensorState.msg.html

(cl:defclass <SensorState> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (sonar
    :reader sonar
    :initarg :sonar
    :type (cl:vector cl:float)
   :initform (cl:make-array 4 :element-type 'cl:float :initial-element 0.0))
   (illumination
    :reader illumination
    :initarg :illumination
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 4 :element-type 'cl:fixnum :initial-element 0))
   (motor_encoder
    :reader motor_encoder
    :initarg :motor_encoder
    :type (cl:vector cl:integer)
   :initform (cl:make-array 4 :element-type 'cl:integer :initial-element 0))
   (button
    :reader button
    :initarg :button
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 7 :element-type 'cl:fixnum :initial-element 0))
   (battery
    :reader battery
    :initarg :battery
    :type cl:float
    :initform 0.0))
)

(cl:defclass SensorState (<SensorState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SensorState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SensorState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name xrobot_msgs-msg:<SensorState> is deprecated: use xrobot_msgs-msg:SensorState instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <SensorState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:header-val is deprecated.  Use xrobot_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'sonar-val :lambda-list '(m))
(cl:defmethod sonar-val ((m <SensorState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:sonar-val is deprecated.  Use xrobot_msgs-msg:sonar instead.")
  (sonar m))

(cl:ensure-generic-function 'illumination-val :lambda-list '(m))
(cl:defmethod illumination-val ((m <SensorState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:illumination-val is deprecated.  Use xrobot_msgs-msg:illumination instead.")
  (illumination m))

(cl:ensure-generic-function 'motor_encoder-val :lambda-list '(m))
(cl:defmethod motor_encoder-val ((m <SensorState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:motor_encoder-val is deprecated.  Use xrobot_msgs-msg:motor_encoder instead.")
  (motor_encoder m))

(cl:ensure-generic-function 'button-val :lambda-list '(m))
(cl:defmethod button-val ((m <SensorState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:button-val is deprecated.  Use xrobot_msgs-msg:button instead.")
  (button m))

(cl:ensure-generic-function 'battery-val :lambda-list '(m))
(cl:defmethod battery-val ((m <SensorState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader xrobot_msgs-msg:battery-val is deprecated.  Use xrobot_msgs-msg:battery instead.")
  (battery m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<SensorState>)))
    "Constants for message type '<SensorState>"
  '((:LEFT_FRONT_MOTOR . 0)
    (:RIGHT_FRONT_MOTOR . 1)
    (:LEFT_BACK_MOTOR . 2)
    (:RIGHT_BACK_MOTOR . 3)
    (:FRONT_SENSOR . 0)
    (:RIGHT_SENSOR . 1)
    (:BACK_SENSOR . 2)
    (:LEFT_SENSOR . 3))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'SensorState)))
    "Constants for message type 'SensorState"
  '((:LEFT_FRONT_MOTOR . 0)
    (:RIGHT_FRONT_MOTOR . 1)
    (:LEFT_BACK_MOTOR . 2)
    (:RIGHT_BACK_MOTOR . 3)
    (:FRONT_SENSOR . 0)
    (:RIGHT_SENSOR . 1)
    (:BACK_SENSOR . 2)
    (:LEFT_SENSOR . 3))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SensorState>) ostream)
  "Serializes a message object of type '<SensorState>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-single-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)))
   (cl:slot-value msg 'sonar))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'illumination))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let* ((signed ele) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    ))
   (cl:slot-value msg 'motor_encoder))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'button))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'battery))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SensorState>) istream)
  "Deserializes a message object of type '<SensorState>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
  (cl:setf (cl:slot-value msg 'sonar) (cl:make-array 4))
  (cl:let ((vals (cl:slot-value msg 'sonar)))
    (cl:dotimes (i 4)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-single-float-bits bits)))))
  (cl:setf (cl:slot-value msg 'illumination) (cl:make-array 4))
  (cl:let ((vals (cl:slot-value msg 'illumination)))
    (cl:dotimes (i 4)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))))
  (cl:setf (cl:slot-value msg 'motor_encoder) (cl:make-array 4))
  (cl:let ((vals (cl:slot-value msg 'motor_encoder)))
    (cl:dotimes (i 4)
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:aref vals i) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))))
  (cl:setf (cl:slot-value msg 'button) (cl:make-array 7))
  (cl:let ((vals (cl:slot-value msg 'button)))
    (cl:dotimes (i 7)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'battery) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SensorState>)))
  "Returns string type for a message object of type '<SensorState>"
  "xrobot_msgs/SensorState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SensorState)))
  "Returns string type for a message object of type 'SensorState"
  "xrobot_msgs/SensorState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SensorState>)))
  "Returns md5sum for a message object of type '<SensorState>"
  "229634ca6575e167dd4ffa0fff8b30f5")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SensorState)))
  "Returns md5sum for a message object of type 'SensorState"
  "229634ca6575e167dd4ffa0fff8b30f5")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SensorState>)))
  "Returns full string definition for message of type '<SensorState>"
  (cl:format cl:nil "########################################~%# CONSTANTS~%########################################~%# Motor~%uint8 LEFT_FRONT_MOTOR  = 0~%uint8 RIGHT_FRONT_MOTOR = 1~%uint8 LEFT_BACK_MOTOR  = 2~%uint8 RIGHT_BACK_MOTOR = 3~%~%# Sensor~%uint8 FRONT_SENSOR  = 0~%uint8 RIGHT_SENSOR = 1~%uint8 BACK_SENSOR  = 2~%uint8 LEFT_SENSOR = 3~%~%########################################~%# Messages~%########################################~%Header  header~%float32[4] sonar~%uint8[4] illumination~%int32[4] motor_encoder~%uint8[7] button~%float32  battery~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SensorState)))
  "Returns full string definition for message of type 'SensorState"
  (cl:format cl:nil "########################################~%# CONSTANTS~%########################################~%# Motor~%uint8 LEFT_FRONT_MOTOR  = 0~%uint8 RIGHT_FRONT_MOTOR = 1~%uint8 LEFT_BACK_MOTOR  = 2~%uint8 RIGHT_BACK_MOTOR = 3~%~%# Sensor~%uint8 FRONT_SENSOR  = 0~%uint8 RIGHT_SENSOR = 1~%uint8 BACK_SENSOR  = 2~%uint8 LEFT_SENSOR = 3~%~%########################################~%# Messages~%########################################~%Header  header~%float32[4] sonar~%uint8[4] illumination~%int32[4] motor_encoder~%uint8[7] button~%float32  battery~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SensorState>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'sonar) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'illumination) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'motor_encoder) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'button) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SensorState>))
  "Converts a ROS message object to a list"
  (cl:list 'SensorState
    (cl:cons ':header (header msg))
    (cl:cons ':sonar (sonar msg))
    (cl:cons ':illumination (illumination msg))
    (cl:cons ':motor_encoder (motor_encoder msg))
    (cl:cons ':button (button msg))
    (cl:cons ':battery (battery msg))
))
