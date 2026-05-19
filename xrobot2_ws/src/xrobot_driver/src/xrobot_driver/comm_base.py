import sys
import time
import enum

from serial import Serial

PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3

def calculate_inter_char(baudrate):
    """calculates the interchar delay from the baudrate"""
    if baudrate <= 19200:
        return 11.0 / baudrate
    else:
        return 0.005
    
def to_data(string_data):
    if PY2:
        return string_data
    else:
        return bytearray(string_data, 'ascii')

class CommResult(enum.IntEnum):
    COMM_SUCCESS = 0  # tx or rx packet communication success
    COMM_PORT_BUSY = -1  # Port is busy (in use)
    COMM_TX_FAIL = -2  # Failed transmit instruction packet
    COMM_RX_FAIL = -3  # Failed get packet
    COMM_TX_ERROR = -4  # Incorrect instruction packet
    COMM_RX_WAITING = -5  # Now recieving packet
    COMM_RX_TIMEOUT = -6  # There is no packet
    COMM_RX_CORRUPT = -7  # Incorrect packet
    COMM_NOT_AVAILABLE = -9  #

class CommBase:
    def __init__(self, serial: Serial, interchar_multiplier=1.5, interframe_multiplier=3.5, t0=None):
        self._serial = serial
        self.use_sw_timeout = False
        self._is_opened = False
        self._interchar_multiplier = interchar_multiplier
        self._interframe_multiplier = interframe_multiplier
        if t0:
            self._t0 = t0
        else:
            self._t0 = calculate_inter_char(self._serial.baudrate)
        self._serial.inter_byte_timeout = self._interchar_multiplier * self._t0
        self.set_timeout(self._interframe_multiplier * self._t0)
        
    def __del__(self):
        """Destructor: close the connection"""
        self.close()

    def __repr__(self) -> str:
        return '{name}<id=0x{id:x}>(port={port})'.format(
                   name=self.__class__.__name__, id=id(self), port=self._serial.port)

    def open(self):
        """open the communication with the slave"""
        if not self._is_opened:
            self._do_open()
            self._is_opened = True

    def close(self):
        """close the communication with the slave"""
        if self._is_opened:
            ret = self._do_close()
            if ret:
                self._is_opened = False

    def _do_open(self):
        """Open the given serial port if not already opened"""
        if not self._serial.is_open:
            self._serial.open()

    def _do_close(self):
        """Close the serial port if still opened"""
        if self._serial.is_open:
            self._serial.close()
            return True

    def set_timeout(self, timeout_in_sec, use_sw_timeout=False):
        """Change the timeout value"""
        self._serial.timeout = timeout_in_sec
        # Use software based timeout in case the timeout functionality provided by the serial port is unreliable
        self.use_sw_timeout = use_sw_timeout

    def _send(self, request):
        """Send request to the slave"""

        self._do_open()
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()

        length = self._serial.write(request)
        self._serial.flush()
        return length

    def _recv(self, expected_length=-1):
        """Receive the response from the slave"""

        response = to_data("")
        start_time = time.time() if self.use_sw_timeout else 0
        while True:
            read_bytes = self._serial.read(expected_length if expected_length > 0 else 1)
            if self.use_sw_timeout:
                read_duration = time.time() - start_time
            else:
                read_duration = 0
            if (not read_bytes) or (read_duration > self._serial.timeout):
                break
            response += read_bytes
            if expected_length >= 0 and len(response) >= expected_length:
                # if the expected number of byte is received consider that the response is done
                # improve performance by avoiding end-of-response detection by timeout
                break
        return response
    
    def _recv_line(self, multiline: bool = True):
        if multiline:
            responses = self._serial.readlines()
            responses = [response.decode().strip() for response in responses]
            return responses
        else:
            responses = self._serial.readline().decode().strip()
        return responses

class ErrorCode(enum.IntEnum):
    EOK = 0         # There is no error
    ERROR = 1       # A generic error happens
    EBUSY = 2       # Busy
    ETIMEOUT = 3    # Time out
    EINVAL = 4      # Invalid parameter
    ERANGE = 5      # Over range 


