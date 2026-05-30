#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import sys
import traceback

import rospy
import serial
from sensor_msgs.msg import JointState
from std_msgs.msg import String


def _as_list(value, default):
    if value is None:
        return list(default)
    if isinstance(value, str):
        value = value.strip()
        if not value:
            return list(default)
        return [item.strip() for item in value.split(",") if item.strip()]
    return list(value)


def _resolve_jetarm_path():
    default_path = os.path.expanduser("~/xrobot2_ws/src/xrobot_arm/jetarm")
    return rospy.get_param("~jetarm_path", default_path)


class ArmServoAngleReader:
    def __init__(self):
        jetarm_path = _resolve_jetarm_path()
        if jetarm_path and jetarm_path not in sys.path:
            sys.path.insert(0, jetarm_path)

        from jetarm.Comm.ftservo import FTServo, scscl, sms_sts
        from jetarm.Comm.comm_base import CommResult

        self.FTServo = FTServo
        self.scscl = scscl
        self.sms_sts = sms_sts
        self.CommResult = CommResult

        self.port_name = rospy.get_param("~port", "/dev/XCOM1")
        self.baud = int(rospy.get_param("~baud", 1000000))
        self.protocol_name = rospy.get_param("~protocol", "sms_sts").lower()
        self.rate_hz = float(rospy.get_param("~rate", 5.0))
        self.frame_id = rospy.get_param("~frame_id", "arm_base")
        self.angle_unit = rospy.get_param("~angle_unit", "degree").lower()
        self.fallback_pos_min = int(rospy.get_param("~fallback_pos_min", 0))
        self.fallback_pos_max = int(rospy.get_param("~fallback_pos_max", 4095))

        servo_ids = _as_list(rospy.get_param("~servo_ids", [1, 2, 3, 4]), [1, 2, 3, 4])
        self.servo_ids = [int(item) for item in servo_ids]

        default_names = ["joint%d" % sid for sid in self.servo_ids]
        self.joint_names = _as_list(rospy.get_param("~joint_names", default_names), default_names)
        if len(self.joint_names) != len(self.servo_ids):
            rospy.logwarn("joint_names length does not match servo_ids; using joint<ID> names")
            self.joint_names = default_names

        angle_mins = _as_list(rospy.get_param("~angle_mins", [-110] * len(self.servo_ids)), [-110] * len(self.servo_ids))
        angle_maxs = _as_list(rospy.get_param("~angle_maxs", [110] * len(self.servo_ids)), [110] * len(self.servo_ids))
        self.angle_mins = self._expand_float_list(angle_mins, -110.0)
        self.angle_maxs = self._expand_float_list(angle_maxs, 110.0)

        self.raw_pub = rospy.Publisher("~raw", String, queue_size=10)
        self.joint_pub = rospy.Publisher("~joint_states", JointState, queue_size=10)

        self.serial_port = None
        self.protocol = None
        self.servos = []

    def _expand_float_list(self, values, fill_value):
        values = [float(item) for item in values]
        if len(values) == 1 and len(self.servo_ids) > 1:
            values = values * len(self.servo_ids)
        while len(values) < len(self.servo_ids):
            values.append(fill_value)
        return values[:len(self.servo_ids)]

    def open(self):
        rospy.loginfo("Opening servo serial port %s at %d", self.port_name, self.baud)
        self.serial_port = serial.Serial(port=self.port_name, baudrate=self.baud, timeout=0.08)

        if self.protocol_name in ("sms", "sts", "sms_sts"):
            self.protocol = self.sms_sts(self.serial_port)
        elif self.protocol_name in ("scs", "scscl"):
            self.protocol = self.scscl(self.serial_port)
        else:
            raise ValueError("Unsupported protocol '%s'. Use sms_sts or scscl." % self.protocol_name)

        self.servos = []
        for idx, sid in enumerate(self.servo_ids):
            servo = self.FTServo(
                self.protocol,
                sid,
                self.angle_mins[idx],
                self.angle_maxs[idx],
            )
            if getattr(servo, "pos_min", 0) == getattr(servo, "pos_max", 0):
                rospy.logwarn(
                    "Servo id=%d angle limit read failed or invalid; fallback encoder range %d..%d",
                    sid,
                    self.fallback_pos_min,
                    self.fallback_pos_max,
                )
                servo.pos_min = self.fallback_pos_min
                servo.pos_max = self.fallback_pos_max
            self.servos.append(servo)

        rospy.loginfo(
            "Servo angle reader ready: ids=%s names=%s protocol=%s",
            self.servo_ids,
            self.joint_names,
            self.protocol_name,
        )

    def close(self):
        if self.serial_port is not None and self.serial_port.is_open:
            self.serial_port.close()

    def spin(self):
        self.open()
        rate = rospy.Rate(self.rate_hz)
        while not rospy.is_shutdown():
            self.publish_once()
            rate.sleep()
        self.close()

    def publish_once(self):
        stamp = rospy.Time.now()
        joint_msg = JointState()
        joint_msg.header.stamp = stamp
        joint_msg.header.frame_id = self.frame_id

        raw_items = []
        for name, sid, servo in zip(self.joint_names, self.servo_ids, self.servos):
            item = {
                "id": sid,
                "name": name,
                "ok": False,
                "angle_deg": None,
                "position": None,
                "ret": None,
                "err": None,
            }
            try:
                position, ret_pos, err_pos = servo.read_pos()
                angle_deg, ret_angle, err_angle = servo.read_angle()

                item.update({
                    "ok": ret_pos == self.CommResult.COMM_SUCCESS and ret_angle == self.CommResult.COMM_SUCCESS,
                    "angle_deg": angle_deg,
                    "position": position,
                    "ret": int(ret_angle.value if hasattr(ret_angle, "value") else ret_angle),
                    "err": int(err_angle),
                })

                if item["ok"]:
                    joint_msg.name.append(name)
                    if self.angle_unit == "radian":
                        joint_msg.position.append(angle_deg * 3.141592653589793 / 180.0)
                    else:
                        joint_msg.position.append(angle_deg)
            except Exception as exc:
                item["error"] = str(exc)
                rospy.logdebug("Failed to read servo %s: %s", sid, traceback.format_exc())

            raw_items.append(item)

        self.joint_pub.publish(joint_msg)
        self.raw_pub.publish(String(data=json.dumps(raw_items, ensure_ascii=False)))


def main():
    rospy.init_node("arm_servo_angle_reader")
    node = ArmServoAngleReader()
    try:
        node.spin()
    except Exception as exc:
        rospy.logerr("arm_servo_angle_reader failed: %s", exc)
        rospy.logdebug(traceback.format_exc())
        node.close()
        raise


if __name__ == "__main__":
    main()
