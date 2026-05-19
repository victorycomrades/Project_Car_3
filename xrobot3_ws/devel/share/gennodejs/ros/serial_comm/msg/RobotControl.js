// Auto-generated. Do not edit!

// (in-package serial_comm.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class RobotControl {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.motor1_speed = null;
      this.motor2_speed = null;
      this.motor3_speed = null;
      this.motor4_speed = null;
      this.joint1_angle = null;
      this.joint2_angle = null;
      this.joint3_angle = null;
      this.wrist_angle = null;
      this.gripper_angle = null;
      this.rest_flag = null;
    }
    else {
      if (initObj.hasOwnProperty('motor1_speed')) {
        this.motor1_speed = initObj.motor1_speed
      }
      else {
        this.motor1_speed = 0;
      }
      if (initObj.hasOwnProperty('motor2_speed')) {
        this.motor2_speed = initObj.motor2_speed
      }
      else {
        this.motor2_speed = 0;
      }
      if (initObj.hasOwnProperty('motor3_speed')) {
        this.motor3_speed = initObj.motor3_speed
      }
      else {
        this.motor3_speed = 0;
      }
      if (initObj.hasOwnProperty('motor4_speed')) {
        this.motor4_speed = initObj.motor4_speed
      }
      else {
        this.motor4_speed = 0;
      }
      if (initObj.hasOwnProperty('joint1_angle')) {
        this.joint1_angle = initObj.joint1_angle
      }
      else {
        this.joint1_angle = 0;
      }
      if (initObj.hasOwnProperty('joint2_angle')) {
        this.joint2_angle = initObj.joint2_angle
      }
      else {
        this.joint2_angle = 0;
      }
      if (initObj.hasOwnProperty('joint3_angle')) {
        this.joint3_angle = initObj.joint3_angle
      }
      else {
        this.joint3_angle = 0;
      }
      if (initObj.hasOwnProperty('wrist_angle')) {
        this.wrist_angle = initObj.wrist_angle
      }
      else {
        this.wrist_angle = 0;
      }
      if (initObj.hasOwnProperty('gripper_angle')) {
        this.gripper_angle = initObj.gripper_angle
      }
      else {
        this.gripper_angle = 0;
      }
      if (initObj.hasOwnProperty('rest_flag')) {
        this.rest_flag = initObj.rest_flag
      }
      else {
        this.rest_flag = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type RobotControl
    // Serialize message field [motor1_speed]
    bufferOffset = _serializer.int16(obj.motor1_speed, buffer, bufferOffset);
    // Serialize message field [motor2_speed]
    bufferOffset = _serializer.int16(obj.motor2_speed, buffer, bufferOffset);
    // Serialize message field [motor3_speed]
    bufferOffset = _serializer.int16(obj.motor3_speed, buffer, bufferOffset);
    // Serialize message field [motor4_speed]
    bufferOffset = _serializer.int16(obj.motor4_speed, buffer, bufferOffset);
    // Serialize message field [joint1_angle]
    bufferOffset = _serializer.int16(obj.joint1_angle, buffer, bufferOffset);
    // Serialize message field [joint2_angle]
    bufferOffset = _serializer.int16(obj.joint2_angle, buffer, bufferOffset);
    // Serialize message field [joint3_angle]
    bufferOffset = _serializer.int16(obj.joint3_angle, buffer, bufferOffset);
    // Serialize message field [wrist_angle]
    bufferOffset = _serializer.int16(obj.wrist_angle, buffer, bufferOffset);
    // Serialize message field [gripper_angle]
    bufferOffset = _serializer.int16(obj.gripper_angle, buffer, bufferOffset);
    // Serialize message field [rest_flag]
    bufferOffset = _serializer.uint8(obj.rest_flag, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type RobotControl
    let len;
    let data = new RobotControl(null);
    // Deserialize message field [motor1_speed]
    data.motor1_speed = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [motor2_speed]
    data.motor2_speed = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [motor3_speed]
    data.motor3_speed = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [motor4_speed]
    data.motor4_speed = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [joint1_angle]
    data.joint1_angle = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [joint2_angle]
    data.joint2_angle = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [joint3_angle]
    data.joint3_angle = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [wrist_angle]
    data.wrist_angle = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [gripper_angle]
    data.gripper_angle = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [rest_flag]
    data.rest_flag = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 19;
  }

  static datatype() {
    // Returns string type for a message object
    return 'serial_comm/RobotControl';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'c444a667b2720c4168deb47dd4f537a3';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    int16 motor1_speed
    int16 motor2_speed
    int16 motor3_speed
    int16 motor4_speed
    int16 joint1_angle
    int16 joint2_angle
    int16 joint3_angle
    int16 wrist_angle
    int16 gripper_angle
    uint8 rest_flag
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new RobotControl(null);
    if (msg.motor1_speed !== undefined) {
      resolved.motor1_speed = msg.motor1_speed;
    }
    else {
      resolved.motor1_speed = 0
    }

    if (msg.motor2_speed !== undefined) {
      resolved.motor2_speed = msg.motor2_speed;
    }
    else {
      resolved.motor2_speed = 0
    }

    if (msg.motor3_speed !== undefined) {
      resolved.motor3_speed = msg.motor3_speed;
    }
    else {
      resolved.motor3_speed = 0
    }

    if (msg.motor4_speed !== undefined) {
      resolved.motor4_speed = msg.motor4_speed;
    }
    else {
      resolved.motor4_speed = 0
    }

    if (msg.joint1_angle !== undefined) {
      resolved.joint1_angle = msg.joint1_angle;
    }
    else {
      resolved.joint1_angle = 0
    }

    if (msg.joint2_angle !== undefined) {
      resolved.joint2_angle = msg.joint2_angle;
    }
    else {
      resolved.joint2_angle = 0
    }

    if (msg.joint3_angle !== undefined) {
      resolved.joint3_angle = msg.joint3_angle;
    }
    else {
      resolved.joint3_angle = 0
    }

    if (msg.wrist_angle !== undefined) {
      resolved.wrist_angle = msg.wrist_angle;
    }
    else {
      resolved.wrist_angle = 0
    }

    if (msg.gripper_angle !== undefined) {
      resolved.gripper_angle = msg.gripper_angle;
    }
    else {
      resolved.gripper_angle = 0
    }

    if (msg.rest_flag !== undefined) {
      resolved.rest_flag = msg.rest_flag;
    }
    else {
      resolved.rest_flag = 0
    }

    return resolved;
    }
};

module.exports = RobotControl;
