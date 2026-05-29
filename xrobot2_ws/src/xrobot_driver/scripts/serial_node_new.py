#!/usr/bin/env python3
"""
xrobot2_ws STM32 串口通信节点（pyserial + SD/ZD 协议版）

替换原来的 rosserial_python 实现：
- 使用 pyserial 直连 STM32，与 xrobot3_ws 的 serial_comm 协议兼容
- SD 帧（STM32 → Jetson）：传感器数据（34 bytes）
- ZD 帧（Jetson → STM32）：电机/机械臂控制指令（23 bytes）
- 无 rosserial 依赖，STM32 未连接时仅报 WARN 而非 ERROR
- 端口不存在时自动重试，不阻塞其他节点启动

协议与 xrobot3_ws/src/serial_comm 完全一致，可直接复用其 STM32 固件。
"""

import rospy
import struct
import time
import threading
from collections import deque

from sensor_msgs.msg import Imu, Range
from std_msgs.msg import String
from geometry_msgs.msg import Quaternion
import math

def _quaternion_from_euler(roll, pitch, yaw):
    """替代 tf.transformations.quaternion_from_euler，避免依赖"""
    cy = math.cos(yaw * 0.5)
    sy = math.sin(yaw * 0.5)
    cp = math.cos(pitch * 0.5)
    sp = math.sin(pitch * 0.5)
    cr = math.cos(roll * 0.5)
    sr = math.sin(roll * 0.5)
    return Quaternion(
        x=sr * cp * cy - cr * sp * sy,
        y=cr * sp * cy + sr * cp * sy,
        z=cr * cp * sy - sr * sp * cy,
        w=cr * cp * cy + sr * sp * sy,
    )

# ---------------------------------------------------------------------------
# CRC16-IBM (与 xrobot3_ws/src/serial_comm/src/crc16.cpp 一致)
# ---------------------------------------------------------------------------
CRC16_IBM_TABLE = [
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
]


def crc16_ibm(data, length):
    """CRC16-IBM 校验，与 STM32 端一致"""
    crc = 0xFFFF
    for i in range(length):
        idx = ((crc ^ data[i]) & 0xFF)
        crc = ((crc >> 8) ^ CRC16_IBM_TABLE[idx]) & 0xFFFF
    return crc


# ---------------------------------------------------------------------------
# 协议常量（与 STM32 固件、xrobot3_ws serial_comm 一致）
# ---------------------------------------------------------------------------
SD_HEADER = b"SD"             # 传感器数据帧头
SD_FRAME_SIZE = 34            # 帧头 2B + 数据 34B + CRC 2B = 38B
ZD_HEADER = b"ZD"             # 控制指令帧头
ZD_FRAME_SIZE = 23            # 帧头 2B + 数据 23B + CRC 2B = 27B

# SD 帧字段偏移（跳过帧头 2B）
# byte 0-3:   line_sensor[4]  (uint8 ×4)
# byte 4-11:  ultrasonic[4]   (uint16 LE ×4)
# byte 12-17: gyro[3]         (int16 LE ×3)
# byte 18-23: accel[3]        (int16 LE ×3)
# byte 24-29: euler[3]        (int16 LE ×3) — 原始 ADC 值，发布时会转换
# byte 30-33: reserved


def parse_sd_frame(frame):
    """解析 SD 传感器数据帧（不含帧头 2B），返回 dict"""
    if len(frame) < 34:
        return None
    # 校验 CRC（跳过帧头 "SD"）
    full = b"SD" + frame[:34]
    received_crc = frame[32] | (frame[33] << 8)
    calc_buf = bytearray(full)
    calc_buf[34] = 0
    calc_buf[35] = 0
    calculated_crc = crc16_ibm(calc_buf, 34)
    if received_crc != calculated_crc:
        return None  # CRC 错误，丢弃

    d = {}
    off = 0
    d["line"] = [frame[off + i] for i in range(4)]
    off += 4
    d["ultrasonic"] = [frame[off + i] | (frame[off + i + 1] << 8) for i in range(0, 8, 2)]
    off += 8
    d["gyro"] = [struct.unpack_from("<h", frame, off + i * 2)[0] for i in range(3)]
    off += 6
    d["accel"] = [struct.unpack_from("<h", frame, off + i * 2)[0] for i in range(3)]
    off += 6
    d["euler"] = [struct.unpack_from("<h", frame, off + i * 2)[0] for i in range(3)]
    return d


def build_zd_frame(m1, m2, m3, m4, j1, j2, j3, wrist, gripper, reset_flag):
    """构建 ZD 控制指令帧（返回 bytes，含帧头 + CRC）"""
    buf = bytearray(25)  # ZD(2) + data(21) + CRC(2) = 25

    buf[0] = ord("Z")
    buf[1] = ord("D")
    off = 2
    for v in (m1, m2, m3, m4, j1, j2, j3, wrist, gripper):
        struct.pack_into("<h", buf, off, int(v))
        off += 2
    buf[off] = int(reset_flag) & 0xFF
    off += 1

    crc = crc16_ibm(buf, off)
    buf[off] = crc & 0xFF
    buf[off + 1] = (crc >> 8) & 0xFF
    return bytes(buf)


# ---------------------------------------------------------------------------
# ROS 串口节点
# ---------------------------------------------------------------------------

class SerialNode:
    def __init__(self):
        self._port_name = rospy.get_param("~port", "/dev/XCOM2")
        self._baud = int(rospy.get_param("~baud", "115200"))
        self._auto_reset_timeout = float(rospy.get_param("~auto_reset_timeout", "0.1"))

        self._serial = None
        self._lock = threading.Lock()
        self._running = True

        # 传感器发布
        self._imu_pub = rospy.Publisher("imu/data", Imu, queue_size=10)
        self._ultrasonic_pub = rospy.Publisher("ultrasonic", Range, queue_size=10)
        self._line_pub = rospy.Publisher("line_sensor", String, queue_size=10)

        # 控制指令订阅：接受 "m1,m2,m3,m4,j1,j2,j3,wrist,gripper,reset" 格式
        rospy.Subscriber("robot_cmd", String, self._cmd_cb)

        # 最后一个命令缓存（即使无新命令也周期性发送）
        self._latest_cmd = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

        rospy.loginfo("serial_node: port=%s baud=%d (SD/ZD protocol, pyserial)",
                      self._port_name, self._baud)

    def _cmd_cb(self, msg):
        """解析 robot_cmd 字符串，更新待发送指令"""
        try:
            parts = [int(x.strip()) for x in msg.data.split(",")]
            if len(parts) >= 10:
                self._latest_cmd = tuple(parts[:10])
        except ValueError:
            rospy.logwarn("serial_node: bad robot_cmd format: %s", msg.data)

    def _open_port(self):
        """尝试打开串口，失败返回 None"""
        try:
            import serial
            ser = serial.Serial(
                port=self._port_name,
                baudrate=self._baud,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.005,      # 5ms 非阻塞读超时
                write_timeout=0.1,
            )
            rospy.loginfo("serial_node: opened %s @ %d", self._port_name, self._baud)
            return ser
        except Exception as e:
            rospy.logwarn("serial_node: cannot open %s — %s (retrying...)", self._port_name, e)
            return None

    def _read_sd_frame(self, ser):
        """从串口读取一个完整的 SD 帧，返回解析后的 dict 或 None"""
        # 找帧头 "SD"
        t0 = time.time()
        state = 0
        while time.time() - t0 < 0.5:  # 500ms 超时
            b = ser.read(1)
            if not b:
                continue
            if state == 0:
                if b == b"S":
                    state = 1
            elif state == 1:
                if b == b"D":
                    state = 2
                    break
                else:
                    state = 0
        if state != 2:
            return None

        # 读剩余数据: 32B data + 2B CRC = 34B
        remaining = 34
        buf = bytearray()
        t0 = time.time()
        while len(buf) < remaining and time.time() - t0 < 0.1:
            chunk = ser.read(remaining - len(buf))
            if chunk:
                buf.extend(chunk)
        if len(buf) < remaining:
            return None

        return parse_sd_frame(bytes(buf))

    def _send_zd_frame(self, ser, cmd):
        """发送 ZD 控制帧"""
        frame = build_zd_frame(*cmd)
        try:
            ser.write(frame)
        except Exception as e:
            rospy.logwarn("serial_node: write failed: %s", e)

    def _publish_sensor(self, data):
        """发布传感器数据到 ROS topic"""
        now = rospy.Time.now()

        # IMU
        imu = Imu()
        imu.header.stamp = now
        imu.header.frame_id = "imu_link"

        gx = data["gyro"][0] * 2000.0 / 32768.0
        gy = data["gyro"][1] * 2000.0 / 32768.0
        gz = data["gyro"][2] * 2000.0 / 32768.0

        ax = data["accel"][0] * 16.0 / 32768.0
        ay = data["accel"][1] * 16.0 / 32768.0
        az = data["accel"][2] * 16.0 / 32768.0

        roll = data["euler"][0] * 180.0 / 32768.0
        pitch = data["euler"][1] * 180.0 / 32768.0
        yaw = data["euler"][2] * 180.0 / 32768.0

        q = _quaternion_from_euler(
            roll * 3.14159265 / 180.0,
            pitch * 3.14159265 / 180.0,
            yaw * 3.14159265 / 180.0,
        )
        imu.orientation = Quaternion(*q)
        imu.angular_velocity.x = gx
        imu.angular_velocity.y = gy
        imu.angular_velocity.z = gz
        imu.linear_acceleration.x = ax
        imu.linear_acceleration.y = ay
        imu.linear_acceleration.z = az
        self._imu_pub.publish(imu)

        # 超声波 (x4)
        for i in range(4):
            r = Range()
            r.header.stamp = now
            r.header.frame_id = "ultrasonic_%d" % (i + 1)
            r.radiation_type = Range.ULTRASOUND
            r.field_of_view = 0.5
            r.min_range = 0.02
            r.max_range = 4.0
            r.range = data["ultrasonic"][i] / 100.0
            self._ultrasonic_pub.publish(r)

        # 巡线传感器
        line = String()
        line.data = "%d,%d,%d,%d" % tuple(data["line"])
        self._line_pub.publish(line)

    def run(self):
        rate = rospy.Rate(50)  # 50Hz
        while not rospy.is_shutdown() and self._running:
            if self._serial is None:
                self._serial = self._open_port()

            if self._serial is not None:
                try:
                    # 先发控制指令
                    self._send_zd_frame(self._serial, self._latest_cmd)

                    # 读传感器数据（非阻塞）
                    data = self._read_sd_frame(self._serial)
                    if data:
                        self._publish_sensor(data)
                except Exception as e:
                    rospy.logwarn("serial_node: I/O error — %s", e)
                    try:
                        self._serial.close()
                    except Exception:
                        pass
                    self._serial = None

            rate.sleep()

    def shutdown(self):
        self._running = False
        if self._serial:
            try:
                self._serial.close()
            except Exception:
                pass


if __name__ == "__main__":
    rospy.init_node("serial_node")
    node = SerialNode()
    try:
        node.run()
    except rospy.ROSInterruptException:
        pass
    finally:
        node.shutdown()
