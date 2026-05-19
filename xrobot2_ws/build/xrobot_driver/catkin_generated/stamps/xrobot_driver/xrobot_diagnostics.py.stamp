import re
import os
import rospy
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus
from xrobot_msgs.msg import VersionInfo, SensorState, ParamConfig
from sensor_msgs.msg import Imu, LaserScan, BatteryState

with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../src/xrobot_driver/__init__.py"), "r") as file:
    regex_version = r'^__version__\s*=\s*[\'"]([^\'"]*)[\'"]'
    version = re.search(regex_version, file.read(), re.MULTILINE).group(1)
    
SOFTWARE_VERSION = version
FIRMWARE_VERSION_MAJOR_NUMBER = version.split(".")[0]
FIRMWARE_VERSION_MINOR_NUMBER = version.split(".")[1]

class XRobotDiagnostic:
    def __init__(self) -> None:
        self.check_version_ = False

        self.chassis_type = rospy.get_param('chassis_type', 2)

        self.diag_pub_ = rospy.Publisher('diagnostics', DiagnosticArray, queue_size=10)
        self.chassis_param_pub_ = rospy.Publisher('chassis_params', ParamConfig, queue_size = 1)
        self.version_info_pub_ = rospy.Publisher('version_info', VersionInfo, queue_size=10)

        self.imu_state_ = DiagnosticStatus(level=DiagnosticStatus.ERROR, name='IMU Sensor', hardware_id="IMU")
        self.motor_state_ = DiagnosticStatus(level=DiagnosticStatus.ERROR, name='Actuator', hardware_id="Motor")
        self.battery_state_ = DiagnosticStatus(level=DiagnosticStatus.ERROR, name='Power System', hardware_id="Battery")
        self.LDS_state_ = DiagnosticStatus(level=DiagnosticStatus.ERROR, name='Lidar Sensor', hardware_id="LDS")

        rospy.Subscriber("imu", Imu, self.img_callback)
        rospy.Subscriber("scan", LaserScan, self.laser_callback)
        rospy.Subscriber("sensor_state", SensorState, self.sensor_callback)
        rospy.Subscriber("firmware_version", VersionInfo, self.firmware_version_callback)
        rospy.Subscriber("battery_state", BatteryState, self.battery_callback)

        # Define Diagnostic array message
        # http://docs.ros.org/api/diagnostic_msgs/html/msg/DiagnosticStatus.html
        self.diag_arr_ = DiagnosticArray()

    def img_callback(self, message):
        self.imu_state_.level = DiagnosticStatus.OK
        self.imu_state_.message = "Good Condition"

    def laser_callback(self, message):
        self.LDS_state_.level = DiagnosticStatus.OK
        self.LDS_state_.message = "Good Condition"

    def sensor_callback(self, message):
        pass

    def firmware_version_callback(self, message):
        major_number, minor_number, patch_number = message.firmware.split(".")

        if self.check_version_ == False:
            if major_number == FIRMWARE_VERSION_MAJOR_NUMBER:
                if minor_number > FIRMWARE_VERSION_MINOR_NUMBER:
                    rospy.logwarn("This firmware(v%s) may not compatible with this software (v%s)", message.firmware.data(), SOFTWARE_VERSION)
                    rospy.logwarn("You can find how to update its in `FAQ` section(https://gitee.com/xrobot-arm)")
            else:
                rospy.logwarn("This firmware(v%s) may not compatible with this software (v%s)", message.firmware.data(), SOFTWARE_VERSION);
                rospy.logwarn("You can find how to update its in `FAQ` section(https://gitee.com/xrobot-arm)")

            # 同步底盘参数
            param = ParamConfig(chassis_type = self.chassis_type,
                                x_speed_scale = 1.0,
                                y_speed_scale = 1.0,
                                w_speed_scale = 1.0)

            self.chassis_param_pub_.publish(param)

            self.check_version_ = True

    def battery_callback(self, message):
        if message.percentage > 0.2:
            self.battery_state_.level = DiagnosticStatus.OK
            self.battery_state_.message = "Good Condition"
        else:
            self.battery_state_.level = DiagnosticStatus.WARN
            self.battery_state_.message = "Charge!!! Charge!!!"

    def run(self):
        # Add timestamp
        self.diag_arr_.header.stamp = rospy.Time.now()

        self.diag_arr_.status.clear()
        self.diag_arr_.status.append(self.imu_state_)
        self.diag_arr_.status.append(self.motor_state_)
        self.diag_arr_.status.append(self.battery_state_)
        self.diag_arr_.status.append(self.LDS_state_)

        self.diag_pub_.publish(self.diag_arr_)

def main():
    rospy.init_node("xrobot_diagnostic")

    diagnostic = XRobotDiagnostic()

    loop_rate = rospy.Rate(1)
    while not rospy.is_shutdown():
        diagnostic.run()
        loop_rate.sleep()

if __name__=="__main__":
    main()