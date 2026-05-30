#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
arm_servo_angle_reader_node.py  — SD/ZD 协议版

通过 XCOM1 (115200 baud) 接收 STM32 发送的 SD 传感器数据帧，
从中提取 4 路舵机编码器值，转换为角度后发布为 JointState。

协议要求 STM32 固件版本 >= 扩展版（SD 帧 42 字节，含 servo1~4_pos）。
"""

import json
import struct
import time
import traceback

import rospy
import serial
from sensor_msgs.msg import JointState
from std_msgs.msg import String

# ---------------------------------------------------------------------------
# CRC-16/ARC (poly=0x8005, init=0x0000, reflected=False)
# 与 STM32 固件 thread_sensor.c 中 crc16_ibm() 一致
# ---------------------------------------------------------------------------
def crc16_arc(data):
    crc = 0x0000
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x8005
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


# ---------------------------------------------------------------------------
# SD 帧常量（与 thread_sensor.h 一致）
# ---------------------------------------------------------------------------
SD_HEADER = b"SD"
# 新版帧：header(2) + data(40) + crc(2) = 44 字节
# 其中数据部分 = line(4) + ultrasonic(8) + gyro(6) + accel(6) + euler(6) + servo(8) = 40
SD_FRAME_SIZE = 42  # 含帧头 "SD" 的总字节数
SD_DATA_BEFORE_SERVO = 32  # servo 字段之前的非 CRC 字节数 (header 2 + data 30)

# 结构体布局（相对帧起始的偏移）:
#   [0]   header[2]    "SD"
#   [2]   line[4]      uint8 ×4
#   [6]   ultrasonic[4] uint16 LE ×4
#   [14]  gyro[3]      int16 LE ×3
#   [20]  accel[3]     int16 LE ×3
#   [26]  euler[3]     int16 LE ×3
#   [32]  servo[4]     int16 LE ×4   ← 新增
#   [40]  crc          uint16 (CRC-16/ARC over bytes 0..39)


def _as_list(value, default):
    if value is None:
        return list(default)
    if isinstance(value, str):
        value = value.strip()
        if not value:
            return list(default)
        return [item.strip() for item in value.split(",") if item.strip()]
    return list(value)


class ArmServoAngleReader:
    def __init__(self):
        self.port_name = rospy.get_param("~port", "/dev/XCOM1")
        self.baud = int(rospy.get_param("~baud", 115200))
        self.rate_hz = float(rospy.get_param("~rate", 20.0))
        self.frame_id = rospy.get_param("~frame_id", "arm_base")
        self.angle_unit = rospy.get_param("~angle_unit", "degree").lower()

        # 舵机 ID 列表（对应 SD 帧中 servo1~4 的顺序）
        servo_ids = _as_list(rospy.get_param("~servo_ids", [1, 2, 3, 4]), [1, 2, 3, 4])
        self.servo_ids = [int(item) for item in servo_ids]
        self.num_servos = len(self.servo_ids)

        # 关节名
        default_names = ["joint%d" % sid for sid in self.servo_ids]
        self.joint_names = _as_list(
            rospy.get_param("~joint_names", default_names), default_names
        )
        if len(self.joint_names) != self.num_servos:
            rospy.logwarn("joint_names length mismatch; using joint<ID> names")
            self.joint_names = default_names

        # 角度范围（机械限位，用于编码器→角度换算）
        angle_mins = _as_list(
            rospy.get_param("~angle_mins", [-110] * self.num_servos),
            [-110] * self.num_servos,
        )
        angle_maxs = _as_list(
            rospy.get_param("~angle_maxs", [110] * self.num_servos),
            [110] * self.num_servos,
        )
        self.angle_mins = self._expand_float_list(angle_mins, -110.0)
        self.angle_maxs = self._expand_float_list(angle_maxs, 110.0)

        # 编码器范围（从舵机 EEPROM 读取或使用默认值）
        enc_mins = _as_list(
            rospy.get_param("~encoder_mins", [0] * self.num_servos),
            [0] * self.num_servos,
        )
        enc_maxs = _as_list(
            rospy.get_param("~encoder_maxs", [4095] * self.num_servos),
            [4095] * self.num_servos,
        )
        self.encoder_mins = self._expand_int_list(enc_mins, 0)
        self.encoder_maxs = self._expand_int_list(enc_maxs, 4095)

        # Publishers
        self.raw_pub = rospy.Publisher("~raw", String, queue_size=10)
        self.joint_pub = rospy.Publisher("~joint_states", JointState, queue_size=10)

        self._serial = None
        self._buf = b""

    def _expand_float_list(self, values, fill_value):
        values = [float(item) for item in values]
        if len(values) == 1 and self.num_servos > 1:
            values = values * self.num_servos
        while len(values) < self.num_servos:
            values.append(fill_value)
        return values[:self.num_servos]

    def _expand_int_list(self, values, fill_value):
        values = [int(item) for item in values]
        if len(values) == 1 and self.num_servos > 1:
            values = values * self.num_servos
        while len(values) < self.num_servos:
            values.append(fill_value)
        return values[:self.num_servos]

    # ------------------------------------------------------------------
    # 串口 & 帧解析
    # ------------------------------------------------------------------
    def open(self):
        rospy.loginfo("Opening STM32 serial port %s at %d baud", self.port_name, self.baud)
        self._serial = serial.Serial(
            port=self.port_name,
            baudrate=self.baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.05,
        )
        rospy.loginfo(
            "Servo angle reader ready: ids=%s names=%s (SD/ZD protocol)",
            self.servo_ids,
            self.joint_names,
        )

    def close(self):
        if self._serial is not None and self._serial.is_open:
            self._serial.close()

    def _read_one_sd_frame(self):
        """从串口读取一个 SD 帧，成功返回 42 字节的 bytes (含 'SD' 帧头)，失败返回 None"""
        # 找帧头 "SD"
        t0 = time.time()
        state = 0
        while time.time() - t0 < 0.3:
            b = self._serial.read(1)
            if not b:
                continue
            if state == 0:
                if b == b"S":
                    state = 1
            elif state == 1:
                if b == b"D":
                    break
                else:
                    state = 0
        if state != 1:
            return None

        # 读剩余 40 字节 (SD 帧总长 42，已读 2)
        remaining = SD_FRAME_SIZE - 2
        buf = bytearray()
        t0 = time.time()
        while len(buf) < remaining and time.time() - t0 < 0.1:
            chunk = self._serial.read(remaining - len(buf))
            if chunk:
                buf.extend(chunk)
        if len(buf) < remaining:
            return None

        return b"SD" + bytes(buf)

    def _parse_sd_frame(self, frame):
        """解析 42 字节 SD 帧，返回 servo 编码器值列表 [s1, s2, s3, s4]，失败返回 None"""
        if len(frame) < SD_FRAME_SIZE:
            return None

        # CRC 校验（CRC-16/ARC over bytes 0..39）
        received_crc = frame[40] | (frame[41] << 8)
        calc_buf = bytearray(frame[:40])
        calculated_crc = crc16_arc(calc_buf)
        if received_crc != calculated_crc:
            rospy.logdebug("SD frame CRC mismatch: got 0x%04X, calc 0x%04X",
                           received_crc, calculated_crc)
            return None

        # 提取舵机编码器值（偏移 32，4 × int16 LE）
        servo_positions = []
        for i in range(self.num_servos):
            off = 32 + i * 2
            if off + 2 <= len(frame):
                pos = struct.unpack_from("<h", frame, off)[0]
                servo_positions.append(pos)

        return servo_positions

    # ------------------------------------------------------------------
    # 编码器 → 角度换算
    # ------------------------------------------------------------------
    def encoder_to_angle(self, idx, pos):
        """编码器值 → 角度（与 SMS_STS 舵机一致）"""
        enc_min = self.encoder_mins[idx]
        enc_max = self.encoder_maxs[idx]
        angle_min = self.angle_mins[idx]
        angle_max = self.angle_maxs[idx]

        if enc_max == enc_min:
            return 0.0

        ratio = float(pos - enc_min) / float(enc_max - enc_min)
        return angle_min + ratio * (angle_max - angle_min)

    # ------------------------------------------------------------------
    # 主循环
    # ------------------------------------------------------------------
    def spin(self):
        self.open()
        rate = rospy.Rate(self.rate_hz)
        while not rospy.is_shutdown():
            self._publish_once()
            rate.sleep()
        self.close()

    def _publish_once(self):
        frame = self._read_one_sd_frame()
        if frame is None:
            return

        positions = self._parse_sd_frame(frame)
        if positions is None or len(positions) < self.num_servos:
            return

        stamp = rospy.Time.now()

        # 构建 JointState
        joint_msg = JointState()
        joint_msg.header.stamp = stamp
        joint_msg.header.frame_id = self.frame_id

        raw_items = []
        for i, sid in enumerate(self.servo_ids):
            pos = positions[i]
            angle_deg = self.encoder_to_angle(i, pos)

            item = {
                "id": sid,
                "name": self.joint_names[i],
                "ok": True,
                "encoder": pos,
                "angle_deg": round(angle_deg, 2),
            }

            joint_msg.name.append(self.joint_names[i])
            if self.angle_unit == "radian":
                joint_msg.position.append(angle_deg * 3.141592653589793 / 180.0)
            else:
                joint_msg.position.append(round(angle_deg, 4))

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
