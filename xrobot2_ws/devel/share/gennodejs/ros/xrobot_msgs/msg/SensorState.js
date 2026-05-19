// Auto-generated. Do not edit!

// (in-package xrobot_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class SensorState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.sonar = null;
      this.illumination = null;
      this.motor_encoder = null;
      this.button = null;
      this.battery = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('sonar')) {
        this.sonar = initObj.sonar
      }
      else {
        this.sonar = new Array(4).fill(0);
      }
      if (initObj.hasOwnProperty('illumination')) {
        this.illumination = initObj.illumination
      }
      else {
        this.illumination = new Array(4).fill(0);
      }
      if (initObj.hasOwnProperty('motor_encoder')) {
        this.motor_encoder = initObj.motor_encoder
      }
      else {
        this.motor_encoder = new Array(4).fill(0);
      }
      if (initObj.hasOwnProperty('button')) {
        this.button = initObj.button
      }
      else {
        this.button = new Array(7).fill(0);
      }
      if (initObj.hasOwnProperty('battery')) {
        this.battery = initObj.battery
      }
      else {
        this.battery = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SensorState
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Check that the constant length array field [sonar] has the right length
    if (obj.sonar.length !== 4) {
      throw new Error('Unable to serialize array field sonar - length must be 4')
    }
    // Serialize message field [sonar]
    bufferOffset = _arraySerializer.float32(obj.sonar, buffer, bufferOffset, 4);
    // Check that the constant length array field [illumination] has the right length
    if (obj.illumination.length !== 4) {
      throw new Error('Unable to serialize array field illumination - length must be 4')
    }
    // Serialize message field [illumination]
    bufferOffset = _arraySerializer.uint8(obj.illumination, buffer, bufferOffset, 4);
    // Check that the constant length array field [motor_encoder] has the right length
    if (obj.motor_encoder.length !== 4) {
      throw new Error('Unable to serialize array field motor_encoder - length must be 4')
    }
    // Serialize message field [motor_encoder]
    bufferOffset = _arraySerializer.int32(obj.motor_encoder, buffer, bufferOffset, 4);
    // Check that the constant length array field [button] has the right length
    if (obj.button.length !== 7) {
      throw new Error('Unable to serialize array field button - length must be 7')
    }
    // Serialize message field [button]
    bufferOffset = _arraySerializer.uint8(obj.button, buffer, bufferOffset, 7);
    // Serialize message field [battery]
    bufferOffset = _serializer.float32(obj.battery, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SensorState
    let len;
    let data = new SensorState(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [sonar]
    data.sonar = _arrayDeserializer.float32(buffer, bufferOffset, 4)
    // Deserialize message field [illumination]
    data.illumination = _arrayDeserializer.uint8(buffer, bufferOffset, 4)
    // Deserialize message field [motor_encoder]
    data.motor_encoder = _arrayDeserializer.int32(buffer, bufferOffset, 4)
    // Deserialize message field [button]
    data.button = _arrayDeserializer.uint8(buffer, bufferOffset, 7)
    // Deserialize message field [battery]
    data.battery = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 47;
  }

  static datatype() {
    // Returns string type for a message object
    return 'xrobot_msgs/SensorState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '229634ca6575e167dd4ffa0fff8b30f5';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    ########################################
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
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SensorState(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.sonar !== undefined) {
      resolved.sonar = msg.sonar;
    }
    else {
      resolved.sonar = new Array(4).fill(0)
    }

    if (msg.illumination !== undefined) {
      resolved.illumination = msg.illumination;
    }
    else {
      resolved.illumination = new Array(4).fill(0)
    }

    if (msg.motor_encoder !== undefined) {
      resolved.motor_encoder = msg.motor_encoder;
    }
    else {
      resolved.motor_encoder = new Array(4).fill(0)
    }

    if (msg.button !== undefined) {
      resolved.button = msg.button;
    }
    else {
      resolved.button = new Array(7).fill(0)
    }

    if (msg.battery !== undefined) {
      resolved.battery = msg.battery;
    }
    else {
      resolved.battery = 0.0
    }

    return resolved;
    }
};

// Constants for message
SensorState.Constants = {
  LEFT_FRONT_MOTOR: 0,
  RIGHT_FRONT_MOTOR: 1,
  LEFT_BACK_MOTOR: 2,
  RIGHT_BACK_MOTOR: 3,
  FRONT_SENSOR: 0,
  RIGHT_SENSOR: 1,
  BACK_SENSOR: 2,
  LEFT_SENSOR: 3,
}

module.exports = SensorState;
