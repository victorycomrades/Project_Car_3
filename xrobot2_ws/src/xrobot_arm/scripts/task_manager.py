#!/usr/bin/env python
import rospy
import math
from std_msgs.msg import Bool, String
from geometry_msgs.msg import PoseStamped, Twist
from tf.transformations import quaternion_from_euler


class TaskManager(object):
    def __init__(self):
        self.state = 'WAIT_START'
        self.task_code = None
        self.batch_list = []
        self.current_batch = 0
        self.current_index = 0
        self.current_color = None
        self.target_object_pose = None
        self.object_color = None
        self.target_ring_pose = None
        self.ring_color = None
        self.started = False
        self.nav_in_progress = False
        self.nav_goal_sent = False
        self.nav_result = None

        self.cmd_pub = rospy.Publisher('cmd_vel', Twist, queue_size=1)
        self.display_pub = rospy.Publisher('openmv/display_text', String, queue_size=1)
        self.status_pub = rospy.Publisher('task_manager/status', String, queue_size=1)
        self.nav_goal_pub = rospy.Publisher('/nav_goal', PoseStamped, queue_size=1)
        self.arm_target_pub = rospy.Publisher('/arm_target_pose', PoseStamped, queue_size=1)
        self.maix_mode_pub = rospy.Publisher('/openmv/mode', String, queue_size=1)

        self._last_maix_mode = None

        rospy.Subscriber('task_start', Bool, self.start_cb)
        rospy.Subscriber('openmv/task_code', String, self.task_code_cb)
        rospy.Subscriber('openmv/object_pose', PoseStamped, self.object_pose_cb)
        rospy.Subscriber('openmv/object_color', String, self.object_color_cb)
        rospy.Subscriber('openmv/color_ring_pose', PoseStamped, self.color_ring_pose_cb)
        rospy.Subscriber('openmv/color_ring_color', String, self.color_ring_color_cb)
        rospy.Subscriber('/nav_status', Bool, self.nav_status_cb)
        rospy.Subscriber('task_manager/set_state', String, self.set_state_cb)
        rospy.Subscriber('/estop', Bool, self.estop_cb)

        self.estop_active = False

        # positions loaded from rosparam, with defaults
        default_positions = {
            'home': {'x': 0.0, 'y': 0.0, 'theta': 0.0},
            'qr': {'x': 1.0, 'y': 0.0, 'theta': 0.0},
            'material': {'x': 2.0, 'y': 0.0, 'theta': 0.0},
            'rough': {'x': 3.0, 'y': 0.0, 'theta': 0.0},
            'temp': {'x': 4.0, 'y': 0.0, 'theta': 0.0}
        }
        self.positions = {}
        for key, default in default_positions.items():
            pos = rospy.get_param('~positions/' + key, default)
            self.positions[key] = {'x': pos['x'], 'y': pos['y'], 'theta': pos['theta']}
        rospy.loginfo('Task positions: %s', self.positions)

        self.phase_start = rospy.Time.now()
        self.timeout = rospy.Duration(30.0)

    def make_pose(self, position_key):
        """Build PoseStamped with proper quaternion from euler yaw."""
        if position_key not in self.positions:
            rospy.logerr('Unknown position key: %s', position_key)
            return None
        pos = self.positions[position_key]
        pose = PoseStamped()
        pose.header.frame_id = 'map'
        pose.header.stamp = rospy.Time.now()
        pose.pose.position.x = pos['x']
        pose.pose.position.y = pos['y']
        pose.pose.position.z = 0.0
        q = quaternion_from_euler(0, 0, pos['theta'])
        pose.pose.orientation.x = q[0]
        pose.pose.orientation.y = q[1]
        pose.pose.orientation.z = q[2]
        pose.pose.orientation.w = q[3]
        return pose

    def publish_nav_goal(self, position_key):
        pose = self.make_pose(position_key)
        if pose is None:
            return
        self.nav_goal_pub.publish(pose)
        self.nav_in_progress = True
        self.nav_goal_sent = True
        self.nav_result = None
        rospy.loginfo('Published nav goal to %s (%.2f, %.2f, %.2f)',
                      position_key, pose.pose.position.x, pose.pose.position.y,
                      self.positions[position_key]['theta'])

    def nav_status_cb(self, msg):
        self.nav_result = msg.data
        self.nav_in_progress = False
        if msg.data:
            rospy.loginfo('Navigation succeeded')
        else:
            rospy.logwarn('Navigation failed')

    def set_state_cb(self, msg):
        new_state = msg.data.strip().upper()
        valid_states = ['WAIT_START', 'GO_QR', 'WAIT_QR', 'GOTO_MATERIAL',
                        'WAIT_MATERIAL', 'ALIGN_PICK', 'PICK_OBJECT',
                        'GO_ROUGH', 'WAIT_ROUGH', 'ALIGN_PLACE_ROUGH',
                        'PLACE_ROUGH', 'PICK_BACK_ROUGH', 'GO_TEMP',
                        'WAIT_TEMP', 'ALIGN_PLACE_TEMP', 'PLACE_TEMP',
                        'RETURN_MATERIAL', 'RETURN_HOME']
        if new_state in valid_states:
            old_state = self.state
            self.state = new_state
            self.phase_start = rospy.Time.now()
            self.nav_in_progress = False
            self.nav_goal_sent = False
            self.nav_result = None
            rospy.loginfo('Force state: %s -> %s', old_state, new_state)
        else:
            rospy.logwarn('Invalid state: %s. Valid: %s', new_state, valid_states)

    def estop_cb(self, msg):
        if msg.data:
            self.estop_active = True
            self.stop_robot()
            self.nav_in_progress = False
            self.publish_status('EMERGENCY STOP activated')
        else:
            self.estop_active = False
            self.publish_status('Emergency stop released')

    def publish_status(self, msg):
        self.status_pub.publish(String(data=msg))
        rospy.loginfo(msg)

    def start_cb(self, msg):
        if msg.data and self.state == 'WAIT_START':
            self.started = True
            self.state = 'GO_QR'
            self.nav_goal_sent = False
            self.nav_result = None
            self.phase_start = rospy.Time.now()
            self.publish_status('Start pressed, go to QR area')

    def task_code_cb(self, msg):
        code = msg.data.strip()
        if self.state in ['GO_QR', 'WAIT_QR', 'WAIT_START'] and code:
            self.task_code = code
            self.batch_list = self.parse_task_code(code)
            self.current_batch = 0
            self.current_index = 0
            self.current_color = self.next_color()
            self.state = 'GOTO_MATERIAL'
            self.nav_goal_sent = False
            self.nav_result = None
            self.phase_start = rospy.Time.now()
            self.set_maix_mode('MODE,IDLE')
            self.publish_status('Task code received: %s' % code)
            self.display_pub.publish(String(data=code))

    def parse_task_code(self, code):
        code = code.strip().lower().replace(' ', '')
        batches = []
        for part in code.split('+'):
            if not part:
                continue
            colors = list(part)
            if len(colors) < 3:
                rospy.logwarn('Batch "%s" has only %d colors, padding with last color' % (part, len(colors)))
                while len(colors) < 3:
                    colors.append(colors[-1] if colors else '1')
            elif len(colors) > 3:
                rospy.logwarn('Batch "%s" has %d colors, truncating to 3' % (part, len(colors)))
                colors = colors[:3]
            batches.append([self.normalize_color(c) for c in colors if c])
        return batches

    def set_maix_mode(self, mode):
        """Send mode switch command to MaixCAM via /openmv/mode topic.
        Avoids resending the same mode repeatedly."""
        if mode == self._last_maix_mode:
            return
        self._last_maix_mode = mode
        self.maix_mode_pub.publish(String(data=mode))
        rospy.loginfo('MaixCAM mode -> %s', mode)

    def normalize_color(self, color):
        color = color.strip().lower()
        if color == '1':
            return 'red'
        if color == '2':
            return 'green'
        if color == '3':
            return 'blue'
        if color == '4':
            return 'yellow'
        return color

    def next_color(self):
        if self.current_batch >= len(self.batch_list):
            return None
        batch = self.batch_list[self.current_batch]
        if self.current_index >= len(batch):
            return None
        return batch[self.current_index]

    def advance_color(self):
        self.current_index += 1
        if self.current_batch < len(self.batch_list):
            batch = self.batch_list[self.current_batch]
            if self.current_index >= len(batch):
                self.current_index = 0
                self.current_batch += 1
        self.current_color = self.next_color()
        return self.current_color

    def object_pose_cb(self, msg):
        if self.current_color and self.object_color == self.current_color:
            self.target_object_pose = msg

    def object_color_cb(self, msg):
        self.object_color = msg.data.strip().lower()
        if self.current_color and self.object_color != self.current_color:
            self.target_object_pose = None

    def color_ring_pose_cb(self, msg):
        if self.current_color and self.ring_color == self.current_color:
            self.target_ring_pose = msg

    def color_ring_color_cb(self, msg):
        self.ring_color = msg.data.strip().lower()
        if self.current_color and self.ring_color != self.current_color:
            self.target_ring_pose = None

    def stop_robot(self):
        twist = Twist()
        self.cmd_pub.publish(twist)

    def drive_forward(self, speed=0.15):
        twist = Twist()
        twist.linear.x = speed
        self.cmd_pub.publish(twist)

    def turn_in_place(self, sign=1, speed=0.35):
        twist = Twist()
        twist.angular.z = sign * speed
        self.cmd_pub.publish(twist)

    def align_to_pose(self, pose, max_distance=80.0):
        if not pose:
            return False
        offset_x = pose.pose.position.x
        distance = pose.pose.position.z if pose.pose.position.z > 0 else abs(pose.pose.position.y)
        if abs(offset_x) > 15.0:
            self.turn_in_place(sign=-1 if offset_x > 0 else 1, speed=0.25)
            return False
        if distance < max_distance:
            self.drive_forward(0.12)
            return False
        self.stop_robot()
        return True

    def target_reached_timeout(self):
        return rospy.Time.now() - self.phase_start > self.timeout

    def transition_nav_state(self, goal_key, next_state):
        """Generic navigation state handler. Returns True when transitioning."""
        if not self.nav_goal_sent:
            self.publish_nav_goal(goal_key)
            return False
        if not self.nav_in_progress:
            if self.nav_result:
                self.state = next_state
                self.phase_start = rospy.Time.now()
                self.nav_goal_sent = False
                return True
            else:
                rospy.logwarn('Nav to %s failed, retrying', goal_key)
                self.nav_goal_sent = False
                return False
        if self.target_reached_timeout():
            self.publish_status('%s navigation timeout, retrying' % goal_key)
            self.nav_goal_sent = False
            self.nav_in_progress = False
        return False

    def run(self):
        rate = rospy.Rate(100)
        while not rospy.is_shutdown():
            if self.estop_active:
                self.stop_robot()
                rate.sleep()
                continue

            if self.state == 'WAIT_START':
                self.set_maix_mode('MODE,IDLE')
                self.publish_status('Waiting for start')

            elif self.state == 'GO_QR':
                self.set_maix_mode('MODE,QR')
                self.transition_nav_state('qr', 'WAIT_QR')

            elif self.state == 'WAIT_QR':
                self.publish_status('Waiting for QR code')
                if self.task_code:
                    self.state = 'GOTO_MATERIAL'
                    self.nav_goal_sent = False
                    self.phase_start = rospy.Time.now()
                elif self.target_reached_timeout():
                    self.publish_status('QR wait timeout, retry navigation')
                    self.state = 'GO_QR'
                    self.nav_goal_sent = False
                    self.phase_start = rospy.Time.now()

            elif self.state == 'GOTO_MATERIAL':
                if self.current_color:
                    self.set_maix_mode('MODE,BLOB,' + self.current_color.upper())
                self.transition_nav_state('material', 'WAIT_MATERIAL')

            elif self.state == 'WAIT_MATERIAL':
                self.publish_status('Waiting for material target %s' % self.current_color)
                if self.target_object_pose is not None:
                    self.state = 'ALIGN_PICK'
                    self.phase_start = rospy.Time.now()
                    self.publish_status('Material target found: %s' % self.current_color)
                elif self.target_reached_timeout():
                    self.publish_status('Material wait timeout, retry navigation')
                    self.state = 'GOTO_MATERIAL'
                    self.nav_goal_sent = False
                    self.phase_start = rospy.Time.now()

            elif self.state == 'ALIGN_PICK':
                self.publish_status('Aligning to object %s' % self.current_color)
                if self.align_to_pose(self.target_object_pose, max_distance=90.0):
                    self.state = 'PICK_OBJECT'
                    self.phase_start = rospy.Time.now()
                    self.publish_status('Aligned for pick')
                elif self.target_reached_timeout():
                    self.publish_status('Pick align timeout, retry scan')
                    self.state = 'WAIT_MATERIAL'
                    self.phase_start = rospy.Time.now()

            elif self.state == 'PICK_OBJECT':
                self.publish_status('Pick object: %s' % self.current_color)
                if self.target_object_pose:
                    self.arm_target_pub.publish(self.target_object_pose)
                self.target_object_pose = None
                self.state = 'GO_ROUGH'
                self.nav_goal_sent = False
                self.phase_start = rospy.Time.now()

            elif self.state == 'GO_ROUGH':
                if self.current_color:
                    self.set_maix_mode('MODE,RING,' + self.current_color.upper())
                self.transition_nav_state('rough', 'WAIT_ROUGH')

            elif self.state == 'WAIT_ROUGH':
                self.publish_status('Waiting for rough ring %s' % self.current_color)
                if self.target_ring_pose is not None:
                    self.state = 'ALIGN_PLACE_ROUGH'
                    self.phase_start = rospy.Time.now()
                    self.publish_status('Rough ring found: %s' % self.current_color)
                elif self.target_reached_timeout():
                    self.publish_status('Rough wait timeout, retry navigation')
                    self.state = 'GO_ROUGH'
                    self.nav_goal_sent = False
                    self.phase_start = rospy.Time.now()

            elif self.state == 'ALIGN_PLACE_ROUGH':
                self.publish_status('Aligning to rough ring %s' % self.current_color)
                if self.align_to_pose(self.target_ring_pose, max_distance=100.0):
                    self.state = 'PLACE_ROUGH'
                    self.phase_start = rospy.Time.now()
                    self.publish_status('Aligned at rough ring')
                elif self.target_reached_timeout():
                    self.publish_status('Rough align timeout, retry')
                    self.state = 'WAIT_ROUGH'
                    self.phase_start = rospy.Time.now()

            elif self.state == 'PLACE_ROUGH':
                self.publish_status('Placing to rough ring %s' % self.current_color)
                if self.target_ring_pose:
                    self.arm_target_pub.publish(self.target_ring_pose)
                self.state = 'PICK_BACK_ROUGH'
                self.phase_start = rospy.Time.now()

            elif self.state == 'PICK_BACK_ROUGH':
                self.publish_status('Pick back from rough ring')
                if self.target_ring_pose:
                    self.arm_target_pub.publish(self.target_ring_pose)
                self.target_ring_pose = None
                self.state = 'GO_TEMP'
                self.nav_goal_sent = False
                self.phase_start = rospy.Time.now()

            elif self.state == 'GO_TEMP':
                if self.current_color:
                    self.set_maix_mode('MODE,RING,' + self.current_color.upper())
                self.transition_nav_state('temp', 'WAIT_TEMP')

            elif self.state == 'WAIT_TEMP':
                self.publish_status('Waiting for temp ring %s' % self.current_color)
                if self.target_ring_pose is not None:
                    self.state = 'ALIGN_PLACE_TEMP'
                    self.phase_start = rospy.Time.now()
                    self.publish_status('Temp ring found: %s' % self.current_color)
                elif self.target_reached_timeout():
                    self.publish_status('Temp wait timeout, retry navigation')
                    self.state = 'GO_TEMP'
                    self.nav_goal_sent = False
                    self.phase_start = rospy.Time.now()

            elif self.state == 'ALIGN_PLACE_TEMP':
                self.publish_status('Aligning to temp ring %s' % self.current_color)
                if self.align_to_pose(self.target_ring_pose, max_distance=100.0):
                    self.state = 'PLACE_TEMP'
                    self.phase_start = rospy.Time.now()
                    self.publish_status('Aligned at temp ring')
                elif self.target_reached_timeout():
                    self.publish_status('Temp align timeout, retry')
                    self.state = 'WAIT_TEMP'
                    self.phase_start = rospy.Time.now()

            elif self.state == 'PLACE_TEMP':
                self.publish_status('Placing to temp ring %s' % self.current_color)
                if self.target_ring_pose:
                    self.arm_target_pub.publish(self.target_ring_pose)
                self.target_ring_pose = None
                next_color = self.advance_color()
                if next_color:
                    self.publish_status('Batch complete, next color: %s' % next_color)
                    self.state = 'RETURN_MATERIAL'
                else:
                    self.publish_status('All batches complete, returning home')
                    self.state = 'RETURN_HOME'
                self.nav_goal_sent = False
                self.phase_start = rospy.Time.now()

            elif self.state == 'RETURN_MATERIAL':
                if self.current_color:
                    self.set_maix_mode('MODE,BLOB,' + self.current_color.upper())
                self.transition_nav_state('material', 'WAIT_MATERIAL')

            elif self.state == 'RETURN_HOME':
                self.set_maix_mode('MODE,IDLE')
                if not self.nav_goal_sent:
                    self.publish_nav_goal('home')
                elif not self.nav_in_progress:
                    self.publish_status('Arrived start area')
                    self.stop_robot()
                    self.state = 'WAIT_START'
                    self.task_code = None
                    self.batch_list = []
                    self.current_batch = 0
                    self.current_index = 0
                    self.current_color = None
                    self.target_object_pose = None
                    self.target_ring_pose = None
                    self.object_color = None
                    self.ring_color = None
                    self.started = False
                    self.nav_goal_sent = False
                elif self.target_reached_timeout():
                    self.publish_status('Home navigation timeout')
                    self.state = 'WAIT_START'
                    self.nav_goal_sent = False

            rate.sleep()


if __name__ == '__main__':
    rospy.init_node('task_manager', anonymous=False)
    manager = TaskManager()
    manager.run()
