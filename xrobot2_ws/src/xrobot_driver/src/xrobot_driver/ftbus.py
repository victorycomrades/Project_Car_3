import time

import rospy

from .comm_base import CommBase, CommResult

LATENCY_TIMER = 50 

BROADCAST_ID = 0xFE  # 254
MAX_ID = 0xFC  # 252
SCS_END = 0
STS_END = 1

TXPACKET_MAX_LEN = 250
RXPACKET_MAX_LEN = 250

# Instruction for SCS Protocol
INST_PING = 1   # 查询
INST_READ = 2   # 读
INST_WRITE = 3  # 写
INST_REG_WRITE = 4 # 异步写
INST_ACTION = 5 # 执行异步写
INST_RESET = 6 # 复位
INST_SYNC_WRITE = 0x83 # 同步写
INST_SYNC_READ = 0x82 # 同步读


# for Protocol Packet
PKT_HEADER0 = 0
PKT_HEADER1 = 1
PKT_ID = 2
PKT_LENGTH = 3
PKT_INSTRUCTION = 4
PKT_ERROR = 4
PKT_PARAMETER0 = 5

# Protocol Error bit
ERRBIT_VOLTAGE = 0x01
ERRBIT_ANGLE = 0x02
ERRBIT_OVERHEAT = 0x04
ERRBIT_OVERELE = 0x08
ERRBIT_OVERLOAD = 0x20

class FtBus(CommBase):
    """XRobot舵机总线通讯底层驱动"""
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.packet_start_time = 0.0
        self.packet_timeout = 0.0
        self.tx_time_per_byte = (1000.0 / self._serial.baudrate) * 10.0
        self.is_using = False

    def set_packet_timeout(self, packet_length):
        self.packet_start_time = self.get_current_time()
        self.packet_timeout = (self.tx_time_per_byte * packet_length) + (self.tx_time_per_byte * 3.0) + LATENCY_TIMER

    def set_packet_timeout_millis(self, msec):
        self.packet_start_time = self.get_current_time()
        self.packet_timeout = msec

    def is_packet_timeout(self):
        if self.get_time_since_start() > self.packet_timeout:
            self.packet_timeout = 0
            return True

        return False

    def get_current_time(self):
        return round(time.time() * 1000000000) / 1000000.0

    def get_time_since_start(self):
        time_since = self.get_current_time() - self.packet_start_time
        if time_since < 0.0:
            self.packet_start_time = self.get_current_time()

        return time_since
    
    def write(self, request):
        return self._send(request)

    def read(self, expected_length=-1):
        return self._recv(expected_length)
    
    def _pkt_error(self, error):
        if error & ERRBIT_VOLTAGE:
            return "[ServoStatus] Input voltage error!"

        if error & ERRBIT_ANGLE:
            return "[ServoStatus] Angle sen error!"

        if error & ERRBIT_OVERHEAT:
            return "[ServoStatus] Overheat error!"

        if error & ERRBIT_OVERELE:
            return "[ServoStatus] OverEle error!"
        
        if error & ERRBIT_OVERLOAD:
            return "[ServoStatus] Overload error!"

        return ""

    def write_packet(self, txpacket):
        checksum = 0

        if self.is_using:
            rospy.logwarn("[%s] Transmit failed, Port is in use.", repr(self))
            return CommResult.COMM_PORT_BUSY
        self.is_using = True

        # check max packet length
        if len(txpacket) > TXPACKET_MAX_LEN:
            self.is_using = False
            rospy.logwarn("[%s] Transmit failed. Exceeds the max package length.", repr(self))
            return CommResult.COMM_TX_ERROR

        # add a checksum to the packet
        for c in txpacket[2: -1]:  # except header, checksum
            checksum += c

        txpacket[-1] = ~checksum & 0xFF

        # tx packet
        written_packet_length = self.write(txpacket)
        if len(txpacket) != written_packet_length:
            self.is_using = False
            rospy.logwarn("[%s] Transmit failed.", repr(self))
            return CommResult.COMM_TX_FAIL

        return CommResult.COMM_SUCCESS
    
    def read_packet(self):
        rxpacket = []
        result = CommResult.COMM_RX_FAIL
        checksum = 0
        rx_length = 0
        wait_length = 6  # minimum length (HEADER0 HEADER1 ID LENGTH ERROR CHKSUM)
        
        while True:
            rxpacket.extend(self.read(wait_length - rx_length))
            rx_length = len(rxpacket)
            if rx_length >= wait_length:
                # find packet header
                for idx in range(0, (rx_length - 1)):
                    if (rxpacket[idx] == 0xFF) and (rxpacket[idx + 1] == 0xFF):
                        break

                if idx == 0:  # found at the beginning of the packet
                    if (rxpacket[PKT_ID] > 0xFD) or \
                        (rxpacket[PKT_LENGTH] > RXPACKET_MAX_LEN) or \
                        (rxpacket[PKT_ERROR] > 0x7F):
                        # unavailable ID or unavailable Length or unavailable Error
                        # remove the first byte in the packet
                        del rxpacket[0]
                        rx_length -= 1
                        continue

                    # re-calculate the exact length of the rx packet
                    if wait_length != (rxpacket[PKT_LENGTH] + PKT_LENGTH + 1):
                        wait_length = rxpacket[PKT_LENGTH] + PKT_LENGTH + 1
                        continue

                    # calculate checksum
                    for c in rxpacket[2: - 1]:  # except header, checksum
                        checksum += c
                    checksum = ~checksum & 0xFF

                    # verify checksum
                    if rxpacket[-1] == checksum:
                        result = CommResult.COMM_SUCCESS
                    else:
                        rospy.logwarn("[%s] Receiving failed. verify checksum error", repr(self))
                        result = CommResult.COMM_RX_CORRUPT
                    break

                else:
                    # remove unnecessary packets
                    del rxpacket[0: idx]
                    rx_length -= idx

            else:
                # check timeout
                if self.is_packet_timeout():
                    if rx_length == 0:
                        rospy.logwarn("[%s] Receiving failed. Timeout error", repr(self))
                        result = CommResult.COMM_RX_TIMEOUT
                    else:
                        rospy.logwarn("[%s] Receiving failed.", repr(self))
                        result = CommResult.COMM_RX_CORRUPT
                    break

        self.is_using = False
        return result, rxpacket

    def execute(self, scs_id, function_code, starting_address=0, expected_length=0, output_value=[]):
        txpacket = [0xFF, 0xFF, scs_id, 0, function_code, 0] # make packet header
        rxpacket = []
        result = CommResult.COMM_TX_FAIL
        data = None
        error = 0
        output_len = len(output_value)
        if function_code == INST_PING or function_code == INST_ACTION or function_code == INST_RESET:
            txpacket[PKT_LENGTH] = 2
        elif function_code == INST_READ:
            txpacket = txpacket + (2) * [0]
            txpacket[PKT_LENGTH] = 4
            txpacket[PKT_PARAMETER0 + 0] = starting_address
            txpacket[PKT_PARAMETER0 + 1] = expected_length
        elif function_code == INST_WRITE or function_code == INST_REG_WRITE:
            txpacket = txpacket + (output_len + 1) * [0]
            txpacket[PKT_LENGTH] = output_len + 3
            txpacket[PKT_PARAMETER0] = starting_address
            txpacket[PKT_PARAMETER0 + 1: -1] = output_value[:]
        elif function_code == INST_SYNC_WRITE:
            if output_len % (expected_length+1):
                result = CommResult.COMM_TX_FAIL
                rospy.logwarn("[%s] Transmit failed. Sync write data length error", repr(self))
                return result, error, data
            txpacket[PKT_ID] = BROADCAST_ID
            txpacket = txpacket + (output_len + 2) * [0]
            txpacket[PKT_LENGTH] = output_len + 4
            txpacket[PKT_PARAMETER0 + 0] = starting_address
            txpacket[PKT_PARAMETER0 + 1] = expected_length
            txpacket[PKT_PARAMETER0 + 2: -1] = output_value[:]
        else:
            result = CommResult.COMM_NOT_AVAILABLE
            rospy.logerr("[%s] Transmit failed. The %d function code is not supported.", repr(self), function_code)
            return result, error, data

        # write packet
        result = self.write_packet(txpacket)
        if result != CommResult.COMM_SUCCESS:
            rospy.logwarn("[%s] Transmit failed.", repr(self))
            return result, error, data

        # (ID == Broadcast ID) == no need to wait for status packet or not available
        if (txpacket[PKT_ID] == BROADCAST_ID):
            self.is_using = False
            return result, error, data

        # set packet timeout
        if txpacket[PKT_INSTRUCTION] == INST_READ:
            self.set_packet_timeout(txpacket[PKT_PARAMETER0 + 1] + 6)
        else:
            self.set_packet_timeout(6)  # HEADER0 HEADER1 ID LENGTH ERROR CHECKSUM

        # read packet
        result, rxpacket = self.read_packet()

        if result == CommResult.COMM_SUCCESS and txpacket[PKT_ID] == rxpacket[PKT_ID]:
            error = rxpacket[PKT_ERROR]
            if function_code == INST_READ:
                data = rxpacket[PKT_PARAMETER0 : -1]
            rospy.logdebug("[%s] Communication success.", repr(self))
        return result, error, data

    