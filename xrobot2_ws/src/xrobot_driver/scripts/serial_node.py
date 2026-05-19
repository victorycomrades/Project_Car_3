#!/usr/bin/env python

import rospy
from std_srvs.srv import Empty, EmptyResponse
from rosserial_python import SerialClient as _SerialClient
from serial import SerialException
from time import sleep

import sys

MINIMUM_RESET_TIME = 30

class SerialClient(_SerialClient):

    def __init__(self, *args, **kwargs):
        # The number of seconds to wait after a sync failure for a sync success before automatically performing a reset.
        # If 0, no reset is performed.
        self.auto_reset_timeout = kwargs.pop('auto_reset_timeout', 0)
        self.lastsync_reset = rospy.Time.now()
        rospy.Service('~reset_xrobot', Empty, self.reset_xrobot)
        super(SerialClient, self).__init__(*args, **kwargs)


    def reset_xrobot(self, *args, **kwargs):
        """
        Forces the XRobot to perform a reset, as though its reset button was pressed.
        """
        with self.read_lock, self.write_lock:
            rospy.loginfo('Beginning XRobot reset on port %s. Closing serial port...' % self.port.portstr)
            self.port.close()
            rospy.loginfo('Reopening serial port...')
            self.port.open()
            rospy.loginfo('XRobot reset complete.')
            self.lastsync_reset = rospy.Time.now()
        self.requestTopics()
        return EmptyResponse()

    def sendDiagnostics(self, level, msg_text):
        super(SerialClient, self).sendDiagnostics(level, msg_text)
        # Reset when we haven't received any data from the Arduino in over N seconds.
        if self.auto_reset_timeout and (rospy.Time.now() - self.last_read).secs >= self.auto_reset_timeout:
            if (rospy.Time.now() - self.lastsync_reset).secs < MINIMUM_RESET_TIME:
                rospy.loginfo('Sync has failed, but waiting for last reset to complete.')
            else:
                rospy.loginfo('Sync has failed for over %s seconds. Initiating automatic reset.' % self.auto_reset_timeout)
                self.reset_xrobot()


if __name__=="__main__":

    rospy.init_node("serial_node")
    rospy.loginfo("ROS Serial Python Node")

    port_name = rospy.get_param('~port','/dev/ttyUSB0')
    baud = int(rospy.get_param('~baud','115200'))

    # Number of seconds of sync failure after which Arduino is auto-reset.
    # 0 = no timeout, auto-reset disabled
    auto_reset_timeout = int(rospy.get_param('~auto_reset_timeout','0'))

    # for systems where pyserial yields errors in the fcntl.ioctl(self.fd, TIOCMBIS, \
    # TIOCM_DTR_str) line, which causes an IOError, when using simulated port
    fix_pyserial_for_test = rospy.get_param('~fix_pyserial_for_test', False)

    # TODO: do we really want command line params in addition to parameter server params?
    sys.argv = rospy.myargv(argv=sys.argv)
    if len(sys.argv) >= 2 :
        port_name  = sys.argv[1]

    while not rospy.is_shutdown():
        rospy.loginfo("Connecting to %s at %d baud" % (port_name, baud))
        try:
            client = SerialClient(port_name, baud, fix_pyserial_for_test=fix_pyserial_for_test, auto_reset_timeout=auto_reset_timeout)
            client.run()
        except KeyboardInterrupt:
            break
        except SerialException:
            sleep(1.0)
            continue
        except OSError:
            sleep(1.0)
            continue
