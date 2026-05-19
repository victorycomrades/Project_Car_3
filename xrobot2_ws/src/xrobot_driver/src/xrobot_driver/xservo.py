import enum
import math
import time
from typing import Union, Tuple
import rospy
import serial

from .ftbus import *
from .comm_base import *

#波特率定义
BAUDRATE_1M = 0
BAUDRATE_500K = 1
BAUDRATE_250K = 2
BAUDRATE_128K = 3
BAUDRATE_115200 = 4
BAUDRATE_76800 = 5
BAUDRATE_57600 = 6
BAUDRATE_38400 = 7

#内存表定义
#-------EPROM(读写)--------
VERSION_MAJOR = 3       # 主版本号，可以用来区分舵机类型
VERSION_MINOR = 4       # 子版本号
SERVO_ID = 5            # ID
BAUDRATE = 6            # 波特率
MIN_ANGLE_LIMIT_L = 9   # 最小角度
MIN_ANGLE_LIMIT_H = 10
MAX_ANGLE_LIMIT_L = 11  # 最大角度
MAX_ANGLE_LIMIT_H = 12
CW_DEAD = 26            # 顺时针死区
CCW_DEAD = 27           # 逆时针死区
SMS_STS_OFS_L = 31      # 位置补偿值，用于改变中位值
SMS_STS_OFS_H = 32
SMS_STS_MODE = 33       # 运行模式：0 为位置伺服模式；1 为电机恒速模

#-------SRAM(读写)--------
TORQUE_ENABLE = 40      # 扭矩开关写 0：关闭扭力输出;写 1：打开扭力输出;写 128：当前位置(56)较正为 2048,同时扭矩开关自动置 0
GOAL_POSITION_L = 42    # 目标位置
GOAL_POSITION_H = 43
GOAL_TIME_L = 44        # 暂不支持
GOAL_TIME_H = 45
GOAL_SPEED_L = 46       # 运行速度
GOAL_SPEED_H = 47
SCS_LOCK = 48       # SCS舵机锁标志 写 0 关闭写入锁,写入 EPROM 地址的值掉电不丢失；写 1 打开写入锁,写入 EPROM
SMS_STS_ACC = 41    # 加速度
SMS_STS_LOCK = 55   # SMS和STS舵机锁标志

#-------SRAM(只读)--------
PRESENT_POSITION_L = 56 # 当前位置
PRESENT_POSITION_H = 57
PRESENT_SPEED_L = 58    # 当前速度
PRESENT_SPEED_H = 59
PRESENT_LOAD_L = 60     # 当前负载
PRESENT_LOAD_H = 61     
PRESENT_VOLTAGE = 62    # 当前电压
PRESENT_TEMPERATURE = 63 # 当前温度
MOVING = 66             # 移动标志 舵机运动时标志为 1,舵机停止运动时为 0
PRESENT_CURRENT_L = 69  # 当前电流
PRESENT_CURRENT_H = 70

DEFAULT_BAUDRATE = 1000000

SCS_SERVO_MAJOR_VER = [5]
SMS_STS_SERVO_MAJOR_VER = [9]

class FtServoType(enum.Enum):
    """总线舵机类型"""
    Unknown = -1
    SCS = 0
    STS = 1
    SMS = 2

class FtServoMode(enum.IntEnum):
    """总线舵机控制模式"""
    Servo = 0
    Speed = 1
    PWM = Speed

class FtServo:
    """总线舵机控制对象.
    
    示例:
        servo1 = FtServo(bus, 1, FtServoType.STS, angle_range=360)
        servo_list = FtServo.scan(ser)
    """
    def __init__(self, bus: FtBus, id: int, type: FtServoType = FtServoType.Unknown, angle_range: int =360):
        self._bus = bus
        self._id = id
        self._type = type
        if self._type == FtServoType.SCS:
            self.scs_end = 1
        elif self._type == FtServoType.STS or self._type == FtServoType.SMS:
            self.scs_end = 0
        elif self._type == FtServoType.Unknown:
            _, self._type = FtServo.ping(bus._serial, id)
        
        self._min_angle = -angle_range/2
        self._max_angle = angle_range/2
        self._angle = 0
        self._encoder = 2048
        self._min_encoder = -1
        self._max_encoder = -1
        self._mode = FtServoMode.Servo

        if not isinstance(bus, FtBus):
            raise RuntimeError("使用的总线对象不是FtBus类")
        if self._max_angle == self._min_angle:
            raise RuntimeError("{} 最大角度({})和最小角度({})不能设置为相同值".format(repr(self), self._max_angle, self._min_angle))

    def __repr__(self) -> str:
        return '{name}({type}, id={id}, mode={mode})'.format(
                   name=self.__class__.__name__, id=self.id, type=self._type, mode=self._mode)
    
    def __precede(self) -> ErrorCode:
        """在控制之前运行.

        _max_encoder 或 _min_encoder 存在 -1 时需要先读取一次编码器值.
        获取舵机当前运行在那个模式.

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 
        """
        if self._max_encoder == -1 or self._min_encoder == -1:
            res, err, data = self._bus.execute(self._id, INST_READ, starting_address = MIN_ANGLE_LIMIT_L, 
                                           expected_length=4)
            if res:
                rospy.logwarn("[{}] 编码器上下限未设置, 通讯可能存在异常.", repr(self))
                return ErrorCode.ERROR
            self._min_encoder = self._makeword(data[0], data[1])
            self._max_encoder = self._makeword(data[2], data[3])
            self.get_mode()
        return ErrorCode.EOK
    
    def _getend(self) -> int:
        return self.scs_end
    
    def _setend(self, e) -> int:
        self.scs_end = e

    def _tohost(self, a, b) -> int:
        if (a & (1<<b)):
            return -(a & ~(1<<b))
        else:
            return a
        
    def _toscs(self, a, b) -> int:
        if (a<0):
            return (-a | (1<<b))
        else:
            return a
        
    def _makeword(self, a, b) -> int:
        if self.scs_end==0:
            return (a & 0xFF) | ((b & 0xFF) << 8)
        else:
            return (b & 0xFF) | ((a & 0xFF) << 8)

    def _makedword(self, a, b) -> int:
        return (a & 0xFFFF) | (b & 0xFFFF) << 16

    def _loword(self, l) -> int:
        return l & 0xFFFF

    def _hiword(self, h) -> int:
        return (h >> 16) & 0xFFFF

    def _lobyte(self, w) -> int:
        if self.scs_end==0:
            return w & 0xFF
        else:
            return (w >> 8) & 0xFF

    def _hibyte(self, w) -> int:
        if self.scs_end==0:
            return (w >> 8) & 0xFF
        else:
            return w & 0xFF
        
    def get_id(self) -> int:
        """获取舵机ID.

        返回:
            int, 
        """
        return self._id
    
    def set_id(self, new_id) -> ErrorCode:
        """设置舵机ID.

        参数:
            new_id: 设置的新ID

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 
        """
        if new_id < 0  or new_id > MAX_ID:
            raise ValueError("id超过支持的最大范围0~{}, 当前输入id为{}".format(MAX_ID, new_id))
        
        if self.unlock_eprom():
            return ErrorCode.ERROR
        if self._bus.execute(self._id, INST_WRITE, starting_address = SERVO_ID, output_value=[new_id])[0]:
            return ErrorCode.ERROR
        # if self.lock_eprom():
        #     return ErrorCode.ERROR
        self._id = new_id
        return ErrorCode.EOK
    
    def enabled(self) -> ErrorCode:
        """舵机使能.

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 
        """
        if self._bus.execute(self._id, INST_WRITE, starting_address = TORQUE_ENABLE, output_value=[1])[0]:
            return ErrorCode.ERROR
        return ErrorCode.EOK

    def disabled(self) -> ErrorCode:
        """舵机失能.

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 
        """
        if self._bus.execute(self._id, INST_WRITE, starting_address = TORQUE_ENABLE, output_value=[0])[0]:
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def get_mode(self) -> FtServoMode:
        """获取舵机模式.

        返回:
            FtServoMode, 返回 FtServoMode.Servo 或 FtServoMode.Speed.
        """
        if self._type == FtServoType.STS or self._type == FtServoType.SMS:
            res, err, data = self._bus.execute(self._id, INST_READ, starting_address = SMS_STS_MODE, expected_length=1)
            if not res:
                self._mode = FtServoMode(data[0])
            return self._mode
        else:
            # raise RuntimeError("{} 舵机类型不支持模式的设置".format(self))
            res, err, data = self._bus.execute(self._id, INST_READ, starting_address = MIN_ANGLE_LIMIT_L, expected_length=4)
            if not res:
                min_encoder = self._makeword(data[0], data[1])
                max_encoder = self._makeword(data[2], data[3])
                if min_encoder == 0 and max_encoder == 0:
                    self._mode = FtServoMode.PWM
                else:
                    self._mode = FtServoMode.Servo
                return self._mode

    def set_mode(self, mode: FtServoMode) -> ErrorCode:
        """设置舵机模式.

        参数:
            mode: 伺服模式(FtServoMode.Servo), 速度模式(FtServoMode.Speed), PWM模式(FtServoMode.PWM)
        """
        if self._type == FtServoType.STS or self._type == FtServoType.SMS:
            self.unlock_eprom()
            res, err, data = self._bus.execute(self._id, INST_WRITE, starting_address = SMS_STS_MODE, output_value=[mode])
            if res:
                return ErrorCode.ERROR
            self._mode = mode
            return ErrorCode.EOK
        else:
            # raise RuntimeError("{} 舵机类型不支持模式的设置".format(self))
            if mode == FtServoMode.PWM:
                if self._bus.execute(self._id, INST_WRITE, starting_address = MIN_ANGLE_LIMIT_L, output_value=[0, 0, 0, 0])[0]:
                    return ErrorCode.ERROR
                self._mode = FtServoMode.PWM
            else:
                if self._min_encoder == 0 and self._max_encoder == 0:
                    self._min_encoder = 0
                    self._max_encoder = 1024
                if self._bus.execute(self._id, INST_WRITE, starting_address = MIN_ANGLE_LIMIT_L, 
                                     output_value=[self._lobyte(self._min_encoder), self._hibyte(self._min_encoder), self._lobyte(self._max_encoder), self._hibyte(self._max_encoder)])[0]:
                    return ErrorCode.ERROR
                self._mode = FtServoMode.Servo
            return ErrorCode.EOK
    
    def get_encoder(self) -> int:
        """获取舵机编码器值.

        返回:
            int: 不同舵机编码器范围会有不同. 如果返回 -1 代码获取失败
        """
        res, err, data = self._bus.execute(self._id, INST_READ, starting_address = PRESENT_POSITION_L, 
                                           expected_length=2)
        return self._makeword(data[0], data[1]) if not res else -1

    def set_encoder(self, value: int) -> ErrorCode:
        """获取舵机编码器值.

        返回:
            int: 编码器值. 如果返回 -1 代码获取失败.
        """
        if not isinstance(value, int):
            raise TypeError()
        if self.__precede():
            raise RuntimeError("__precede")
        if (value < self._min_encoder or value > self._max_encoder):
            raise ValueError("encoder超过取值范围{}~{}, 当前输入为{}".format(self._min_encoder, self._max_encoder, value))
        if not (self._mode == FtServoMode.Servo):
            raise RuntimeError("当前不处于伺服模式")

        if self._bus.execute(self._id, INST_WRITE, starting_address = GOAL_POSITION_L, 
                          output_value=[self._lobyte(value), self._hibyte(value)])[0]:
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def revise_encoder(self) -> ErrorCode:
        """将编码器当前位置重置为2048.

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.

        抛出:
            RuntimeError: SCS舵机无法使用此功能函数
        """
        if self._type == FtServoType.SCS:
            raise RuntimeError("SCS舵机无法使用此功能函数")
        
        if self._bus.execute(self._id, INST_WRITE, starting_address = TORQUE_ENABLE, output_value=[128])[0]:
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def get_min_encoder(self) -> int:
        """获取编码器最小值.

        返回:
            int, 编码器最小值, 如果返回 -1 代码获取失败.
        """
        res, err, data = self._bus.execute(self._id, INST_READ, starting_address = MIN_ANGLE_LIMIT_L, 
                                           expected_length=2)
        self._min_encoder = self._makeword(data[0], data[1]) if not res else -1
        return self._min_encoder

    def get_max_encoder(self) -> int:
        """获取编码器最大值.

        返回:
            int, 编码器最大值, 如果返回 -1 代码获取失败.
        """
        res, err, data = self._bus.execute(self._id, INST_READ, starting_address = MAX_ANGLE_LIMIT_L, 
                                           expected_length=2)
        self._max_encoder = self._makeword(data[0], data[1]) if not res else -1
        return self._max_encoder
    
    def get_angle(self) -> float:
        """获取角度值.

        根据编码器值计算角度值.

        返回:
            float, 角度值. 
        """
        if not self.__precede():
            pos = self.get_encoder()
            if pos == -1:
                return 0.0
            angle = round(((self._max_angle - self._min_angle) * (pos - self._min_encoder) /
                        (self._max_encoder - self._min_encoder) + self._min_angle), 2)
            return angle
        return 0.0

    def set_angle(self, value) -> ErrorCode:
        """设置角度值.

        参数:
            value: 写入的角度值

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 
        
        抛出:
            ValueError: 输入的角度值不在范围内
        """
        if value < self._min_angle or value > self._max_angle:
            raise ValueError("angle超过取值范围{}~{}, 当前输入为{}".format(self._min_angle, self._max_angle, value))
        
        if not self.__precede():
            pos = round(((self._max_encoder - self._min_encoder) * (value - self._min_angle) /
                        (self._max_angle - self._min_angle) + self._min_encoder))
            return self.set_encoder(pos)
        return ErrorCode.ERROR
    
    def get_min_angle(self) -> float:
        """获取最小角度值.

        返回:
            float
        """
        return self._min_angle
    
    def get_max_angle(self) -> float:
        """获取最大角度值.

        返回:
            float
        """
        return self._max_angle

    def get_radian(self) -> float:
        """获取弧度值.

        返回:
            float
        """
        return math.radians(self.get_angle())
    
    def set_radian(self, value) -> ErrorCode:
        """设置弧度值.

        参数:
            value: 写入的弧度值

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 
        """
        return self.set_angle(math.degrees(value))
    
    def get_min_radian(self) -> float:
        """获取最小弧度值.

        返回:
            float
        """
        return math.radians(self.get_min_angle())
    
    def get_max_radian(self) -> float:
        """获取最大弧度值.

        返回:
            float
        """
        return math.radians(self.get_max_angle())

    def get_speed(self) -> int:
        """获取速度值.

        返回:
            float
        """
        res, err, data = self._bus.execute(self._id, INST_READ, starting_address = PRESENT_SPEED_L, 
                                           expected_length=2)
        spd = self._makeword(data[0], data[1]) if not res else 0
        return self._tohost(spd, 15)

    def set_speed(self, value):
        """设置速度值.

        参数:
            value: 写入的速度值, 取值范围-32,767~32,767

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 

        抛出:
            ValueError: 输入的速度值不在范围内
        """
        if value < -32767 or value > 32767:
            raise ValueError("speed超过取值范围-32,767~32,767")

        value = self._toscs(value, 15)
        if self._bus.execute(self._id, INST_WRITE, starting_address = GOAL_SPEED_L, 
                          output_value=[self._lobyte(value), self._hibyte(value)])[0]:
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def get_acceleration(self) -> int:
        """获取加速度值.

        返回:
            int, 加速度值. 

        抛出:
            RuntimeError: 舵机不支持加速度的设置
        """
        if not (self._type == FtServoType.STS or self._type == FtServoType.SMS):
            raise RuntimeError("{} 舵机类型不支持加速度的设置".format(self))

        res, err, data = self._bus.execute(self._id, INST_READ, starting_address = SMS_STS_ACC, expected_length = 1)
        return data[0] if not res else 0
            

    def set_acceleration(self, acc: int) -> ErrorCode:
        """设置加速度值.

        参数:
            acc: 写入的加速度值, 取值范围0~255

        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败. 

        抛出:
            ValueError: 输入的加速度值不在范围内
            RuntimeError: 舵机不支持加速度的设置
        """
        if acc < 0 or acc > 255:
            raise ValueError("acc超过取值范围0~255, 当前输入为{}")
        if not (self._type == FtServoType.STS or self._type == FtServoType.SMS):
            raise RuntimeError("{} 舵机类型不支持加速度的设置".format(self))
        
        if self._bus.execute(self._id, INST_WRITE, starting_address = SMS_STS_ACC, output_value=[acc])[0]:
            return ErrorCode.ERROR
        return ErrorCode.EOK
            
        
    def is_moving(self) -> bool:
        """舵机是否正在移动.

        返回:
            bool, 返回 True 正在移动
        """
        res, err, data = self._bus.execute(self._id, INST_READ, starting_address = MOVING, expected_length=1)
        if res:
            return False
        return bool(data[0])
    
    def wait_for_move(self, timeout=5):
        """等待舵机移动结束.

        参数:
            timeout: 超时时间
        """
        t1 = time.time()
        while True:
            time.sleep(0.1)
            if time.time()-t1 > timeout or not self.moving:
                rospy.logwarn("移动超时")
                break

    id = property(get_id, set_id, doc="Servo ID")
    mode = property(get_mode, set_mode, doc="Servo mode")
    encoder = property(get_encoder, set_encoder, doc="Servo encoder")
    min_encoder = property(get_min_encoder, doc="Servo Min encoder")
    max_encoder = property(get_max_encoder, doc="Servo Max encoder")
    angle = property(get_angle, set_angle, doc="Servo angle")
    min_angle = property(get_min_angle, doc="Servo Min angle")
    max_angle = property(get_max_angle, doc="Servo Max angle")
    radian = property(get_radian, set_radian, doc="Servo radian")
    min_radian = property(get_min_radian, doc="Servo Min radian")
    max_radian = property(get_max_radian, doc="Servo Max radian")
    speed = property(get_speed, set_speed, doc="Servo speed")
    acc = property(get_acceleration, set_acceleration, doc="Servo acceleration")
    moving = property(is_moving, doc="Servo is moving")

    def lock_eprom(self):
        """EPROM 上锁.
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.
        """
        if self._type == FtServoType.SCS:
            eprom_address = SCS_LOCK
        elif self._type == FtServoType.STS or self._type == FtServoType.SMS:
            eprom_address = SMS_STS_LOCK
        res, _, _, = self._bus.execute(self._id, INST_WRITE, starting_address = eprom_address, output_value=[1])
        if res:
            rospy.logwarn("[{}] lock_eprom失败", repr(self))
            return ErrorCode.ERROR
        return ErrorCode.EOK
        
    def unlock_eprom(self) -> ErrorCode:
        """EPROM 解锁.
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.
        """
        if self._type == FtServoType.SCS:
            eprom_address = SCS_LOCK
        elif self._type == FtServoType.STS or self._type == FtServoType.SMS:
            eprom_address = SMS_STS_LOCK
        if self._bus.execute(self._id, INST_WRITE, starting_address = eprom_address, output_value=[0])[0]:
            rospy.logwarn("[{}] unlock_eprom失败", repr(self))
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def write_pwm(self, time: int) -> ErrorCode:
        """PWM模式下控制函数.

        参数:
            time: 时间细分范围(0-2047) 1024与0为停止, >1024 正转, <1024 反转
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.

        抛出:
            RuntimeError: 舵机类型不是 SCS, 当前不处于PWM模式
        """
        if self.__precede():
            raise RuntimeError("__precede")
        if not (self._type == FtServoType.SCS):
            raise RuntimeError("该方法只支持 SCS 类型舵机")
        if not (self._mode == FtServoMode.PWM):
            raise RuntimeError("当前不处于PWM模式")

        value = self._toscs(time, 10)
        if self._bus.execute(self._id, INST_WRITE, starting_address = GOAL_TIME_L, output_value=[self._lobyte(value), self._hibyte(value)])[0]:
            rospy.logwarn("[{}] write_pwm 操作失败", repr(self))
            return ErrorCode.ERROR
            
        return ErrorCode.EOK
    
    def write_speed(self, speed: int, acc: int = 0) -> ErrorCode:
        """速度模式下控制函数.

        参数:
            speed: 速度
            acc: 加速的
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.

        抛出:
            RuntimeError: 舵机类型不是 STS/SMS, 当前不处于速度模式
        """
        if self.__precede():
            raise RuntimeError("__precede")
        if not (self._type == FtServoType.STS or self._type == FtServoType.SMS):
            raise TypeError("该方法支持 STS/SMS 类型舵机")
        if not (self._mode == FtServoMode.Speed):
            raise RuntimeError("当前不处于速度模式")

        speed = self._toscs(speed, 15)
        txpacket = [acc, 0, 0, 0, 0, self._lobyte(speed), self._hibyte(speed)]
        if self._bus.execute(self._id, INST_WRITE, starting_address = SMS_STS_ACC, output_value=txpacket)[0]:
            rospy.logwarn("[{}] write_speed 操作失败", repr(self))
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def reset_reg(self):
        if self._bus.execute(self._id, INST_RESET)[0]:
            rospy.logwarn("[{}] reset_reg 操作失败", repr(self))
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def async_write(self, start_address: int, data: list) -> ErrorCode:
        """异步写指令, 写入后需要用 reg_action() 执行.

        参数:
            start_address: 起始地址
            data: 写入的数据
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.
        """
        if self._bus.execute(self._id, INST_REG_WRITE, starting_address = start_address, output_value=data)[0]:
            rospy.logwarn("[{}] 异步写指令失败", repr(self))
            return ErrorCode.ERROR
        return ErrorCode.EOK
    
    def async_write_pos(self, position: int=0, speed: int=0, acc: int=0, time: int=0):
        """异步写入位置速度加速度.

        参数:
            position: 编码器值
            speed: 速度
            acc: 加速度(STS/SMS舵机有效)
            time: 时间(SCS舵机有效)
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.
        """

        if self._type == FtServoType.STS or self._type == FtServoType.SMS:
            txpacket = [acc, self._lobyte(position), self._hibyte(position), 0, 0, self._lobyte(speed), self._hibyte(speed)]
            return self.async_write(SMS_STS_ACC, txpacket)
        else:
            time = self._toscs(time, 10)
            txpacket = [self._lobyte(position), self._hibyte(position), self._lobyte(time), self._hibyte(time), self._lobyte(speed), self._hibyte(speed)]
            return self.async_write(GOAL_POSITION_L, txpacket)
    
    @staticmethod
    def async_action(bus) -> ErrorCode:
        """执行异步控制.

        参数:
            bus: 
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.
        """
        _, _, _, = bus.execute(BROADCAST_ID, INST_ACTION)
        return ErrorCode.EOK
        
    @staticmethod
    def ping(port, id: int, baud=DEFAULT_BAUDRATE) -> Tuple[ErrorCode, FtServoType]:
        """查找总线上是否存在此ID的设备.

        参数:
            port: 串口对象
            id: 舵机ID
        
        返回:
            ErrorCode, 如果返回 ErrorCode.ERROR 函数执行失败.
            servo_type, 舵机类型
        """
        servo_type = FtServoType.Unknown
        if id < 0 or id > BROADCAST_ID:
            return ErrorCode.EINVAL, servo_type
        
        bus = FtBus(serial)
        if bus.execute(id, INST_PING)[0]:
            return ErrorCode.ERROR, servo_type
        else:
            _, _, data = bus.execute(id, INST_READ, starting_address = VERSION_MAJOR, expected_length=2)
            major_version, minor_version = data[0], data[1]
            if major_version in SCS_SERVO_MAJOR_VER:
                servo_type = FtServoType.SCS
            elif major_version in SMS_STS_SERVO_MAJOR_VER:
                servo_type = FtServoType.STS
        return ErrorCode.EOK, servo_type
    
    @staticmethod
    def scan(port, ids: Union[list, None]=None, baud=DEFAULT_BAUDRATE) -> list:
        """扫码总线上的舵机设备.

        参数:
            serial: 串口对象
            ids: 便利的ID列表
        
        返回:
            list, 保存所有舵机的列表
        """
        servo_list = []
        bus = FtBus(port, baud)
        if ids == None:
            ids = range(MAX_ID)
        for i in ids:
            res, type = FtServo.ping(port, baud=baud, id=i)
            if not res:
                servo_list.append(FtServo(bus, i, type))

        return servo_list