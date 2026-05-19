// Auto-generated. Do not edit!

// (in-package xrobot_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class ParamConfig {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.chassis_type = null;
      this.x_speed_scale = null;
      this.y_speed_scale = null;
      this.w_speed_scale = null;
    }
    else {
      if (initObj.hasOwnProperty('chassis_type')) {
        this.chassis_type = initObj.chassis_type
      }
      else {
        this.chassis_type = 0;
      }
      if (initObj.hasOwnProperty('x_speed_scale')) {
        this.x_speed_scale = initObj.x_speed_scale
      }
      else {
        this.x_speed_scale = 0.0;
      }
      if (initObj.hasOwnProperty('y_speed_scale')) {
        this.y_speed_scale = initObj.y_speed_scale
      }
      else {
        this.y_speed_scale = 0.0;
      }
      if (initObj.hasOwnProperty('w_speed_scale')) {
        this.w_speed_scale = initObj.w_speed_scale
      }
      else {
        this.w_speed_scale = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type ParamConfig
    // Serialize message field [chassis_type]
    bufferOffset = _serializer.uint8(obj.chassis_type, buffer, bufferOffset);
    // Serialize message field [x_speed_scale]
    bufferOffset = _serializer.float32(obj.x_speed_scale, buffer, bufferOffset);
    // Serialize message field [y_speed_scale]
    bufferOffset = _serializer.float32(obj.y_speed_scale, buffer, bufferOffset);
    // Serialize message field [w_speed_scale]
    bufferOffset = _serializer.float32(obj.w_speed_scale, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type ParamConfig
    let len;
    let data = new ParamConfig(null);
    // Deserialize message field [chassis_type]
    data.chassis_type = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [x_speed_scale]
    data.x_speed_scale = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [y_speed_scale]
    data.y_speed_scale = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [w_speed_scale]
    data.w_speed_scale = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 13;
  }

  static datatype() {
    // Returns string type for a message object
    return 'xrobot_msgs/ParamConfig';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'f701675f55a47d8728e25b639a888073';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    ########################################
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
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new ParamConfig(null);
    if (msg.chassis_type !== undefined) {
      resolved.chassis_type = msg.chassis_type;
    }
    else {
      resolved.chassis_type = 0
    }

    if (msg.x_speed_scale !== undefined) {
      resolved.x_speed_scale = msg.x_speed_scale;
    }
    else {
      resolved.x_speed_scale = 0.0
    }

    if (msg.y_speed_scale !== undefined) {
      resolved.y_speed_scale = msg.y_speed_scale;
    }
    else {
      resolved.y_speed_scale = 0.0
    }

    if (msg.w_speed_scale !== undefined) {
      resolved.w_speed_scale = msg.w_speed_scale;
    }
    else {
      resolved.w_speed_scale = 0.0
    }

    return resolved;
    }
};

// Constants for message
ParamConfig.Constants = {
  DIFFERENTIAL_DRIVE: 0,
  ACKERMANN: 1,
  MECANUM: 2,
}

module.exports = ParamConfig;
