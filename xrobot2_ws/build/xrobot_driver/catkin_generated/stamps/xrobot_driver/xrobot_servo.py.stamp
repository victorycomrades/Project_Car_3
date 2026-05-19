import rospy
from xrobot_driver import xservo

class XRobotServo:
    def __init__(self, port=None, baud=57600, timeout=5.0) -> None:
        self._port=port
        self.timeout = timeout

def main():
    rospy.init_node("xrobot_servo")

    port_name = rospy.get_param('~port','/dev/ttyUSB0')
    baud = int(rospy.get_param('~baud','115200'))

    xrobot_servo = XRobotServo(port_name, baud)

    rospy.spin()
    

if __name__=="__main__":
    main()