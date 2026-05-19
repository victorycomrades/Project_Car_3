; Auto-generated. Do not edit!


(cl:in-package serial_comm-msg)


;//! \htmlinclude RobotControl.msg.html

(cl:defclass <RobotControl> (roslisp-msg-protocol:ros-message)
  ((motor1_speed
    :reader motor1_speed
    :initarg :motor1_speed
    :type cl:fixnum
    :initform 0)
   (motor2_speed
    :reader motor2_speed
    :initarg :motor2_speed
    :type cl:fixnum
    :initform 0)
   (motor3_speed
    :reader motor3_speed
    :initarg :motor3_speed
    :type cl:fixnum
    :initform 0)
   (motor4_speed
    :reader motor4_speed
    :initarg :motor4_speed
    :type cl:fixnum
    :initform 0)
   (joint1_angle
    :reader joint1_angle
    :initarg :joint1_angle
    :type cl:fixnum
    :initform 0)
   (joint2_angle
    :reader joint2_angle
    :initarg :joint2_angle
    :type cl:fixnum
    :initform 0)
   (joint3_angle
    :reader joint3_angle
    :initarg :joint3_angle
    :type cl:fixnum
    :initform 0)
   (wrist_angle
    :reader wrist_angle
    :initarg :wrist_angle
    :type cl:fixnum
    :initform 0)
   (gripper_angle
    :reader gripper_angle
    :initarg :gripper_angle
    :type cl:fixnum
    :initform 0)
   (rest_flag
    :reader rest_flag
    :initarg :rest_flag
    :type cl:fixnum
    :initform 0))
)

(cl:defclass RobotControl (<RobotControl>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <RobotControl>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'RobotControl)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name serial_comm-msg:<RobotControl> is deprecated: use serial_comm-msg:RobotControl instead.")))

(cl:ensure-generic-function 'motor1_speed-val :lambda-list '(m))
(cl:defmethod motor1_speed-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:motor1_speed-val is deprecated.  Use serial_comm-msg:motor1_speed instead.")
  (motor1_speed m))

(cl:ensure-generic-function 'motor2_speed-val :lambda-list '(m))
(cl:defmethod motor2_speed-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:motor2_speed-val is deprecated.  Use serial_comm-msg:motor2_speed instead.")
  (motor2_speed m))

(cl:ensure-generic-function 'motor3_speed-val :lambda-list '(m))
(cl:defmethod motor3_speed-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:motor3_speed-val is deprecated.  Use serial_comm-msg:motor3_speed instead.")
  (motor3_speed m))

(cl:ensure-generic-function 'motor4_speed-val :lambda-list '(m))
(cl:defmethod motor4_speed-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:motor4_speed-val is deprecated.  Use serial_comm-msg:motor4_speed instead.")
  (motor4_speed m))

(cl:ensure-generic-function 'joint1_angle-val :lambda-list '(m))
(cl:defmethod joint1_angle-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:joint1_angle-val is deprecated.  Use serial_comm-msg:joint1_angle instead.")
  (joint1_angle m))

(cl:ensure-generic-function 'joint2_angle-val :lambda-list '(m))
(cl:defmethod joint2_angle-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:joint2_angle-val is deprecated.  Use serial_comm-msg:joint2_angle instead.")
  (joint2_angle m))

(cl:ensure-generic-function 'joint3_angle-val :lambda-list '(m))
(cl:defmethod joint3_angle-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:joint3_angle-val is deprecated.  Use serial_comm-msg:joint3_angle instead.")
  (joint3_angle m))

(cl:ensure-generic-function 'wrist_angle-val :lambda-list '(m))
(cl:defmethod wrist_angle-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:wrist_angle-val is deprecated.  Use serial_comm-msg:wrist_angle instead.")
  (wrist_angle m))

(cl:ensure-generic-function 'gripper_angle-val :lambda-list '(m))
(cl:defmethod gripper_angle-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:gripper_angle-val is deprecated.  Use serial_comm-msg:gripper_angle instead.")
  (gripper_angle m))

(cl:ensure-generic-function 'rest_flag-val :lambda-list '(m))
(cl:defmethod rest_flag-val ((m <RobotControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader serial_comm-msg:rest_flag-val is deprecated.  Use serial_comm-msg:rest_flag instead.")
  (rest_flag m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <RobotControl>) ostream)
  "Serializes a message object of type '<RobotControl>"
  (cl:let* ((signed (cl:slot-value msg 'motor1_speed)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'motor2_speed)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'motor3_speed)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'motor4_speed)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'joint1_angle)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'joint2_angle)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'joint3_angle)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'wrist_angle)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'gripper_angle)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'rest_flag)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <RobotControl>) istream)
  "Deserializes a message object of type '<RobotControl>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'motor1_speed) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'motor2_speed) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'motor3_speed) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'motor4_speed) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'joint1_angle) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'joint2_angle) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'joint3_angle) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'wrist_angle) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'gripper_angle) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'rest_flag)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<RobotControl>)))
  "Returns string type for a message object of type '<RobotControl>"
  "serial_comm/RobotControl")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'RobotControl)))
  "Returns string type for a message object of type 'RobotControl"
  "serial_comm/RobotControl")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<RobotControl>)))
  "Returns md5sum for a message object of type '<RobotControl>"
  "c444a667b2720c4168deb47dd4f537a3")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'RobotControl)))
  "Returns md5sum for a message object of type 'RobotControl"
  "c444a667b2720c4168deb47dd4f537a3")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<RobotControl>)))
  "Returns full string definition for message of type '<RobotControl>"
  (cl:format cl:nil "int16 motor1_speed~%int16 motor2_speed~%int16 motor3_speed~%int16 motor4_speed~%int16 joint1_angle~%int16 joint2_angle~%int16 joint3_angle~%int16 wrist_angle~%int16 gripper_angle~%uint8 rest_flag~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'RobotControl)))
  "Returns full string definition for message of type 'RobotControl"
  (cl:format cl:nil "int16 motor1_speed~%int16 motor2_speed~%int16 motor3_speed~%int16 motor4_speed~%int16 joint1_angle~%int16 joint2_angle~%int16 joint3_angle~%int16 wrist_angle~%int16 gripper_angle~%uint8 rest_flag~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <RobotControl>))
  (cl:+ 0
     2
     2
     2
     2
     2
     2
     2
     2
     2
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <RobotControl>))
  "Converts a ROS message object to a list"
  (cl:list 'RobotControl
    (cl:cons ':motor1_speed (motor1_speed msg))
    (cl:cons ':motor2_speed (motor2_speed msg))
    (cl:cons ':motor3_speed (motor3_speed msg))
    (cl:cons ':motor4_speed (motor4_speed msg))
    (cl:cons ':joint1_angle (joint1_angle msg))
    (cl:cons ':joint2_angle (joint2_angle msg))
    (cl:cons ':joint3_angle (joint3_angle msg))
    (cl:cons ':wrist_angle (wrist_angle msg))
    (cl:cons ':gripper_angle (gripper_angle msg))
    (cl:cons ':rest_flag (rest_flag msg))
))
