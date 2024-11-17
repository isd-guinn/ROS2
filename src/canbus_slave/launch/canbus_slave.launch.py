##launch file
from launch import LaunchDescription
import launch_ros.actions 

def generate_launch_description():
    return LaunchDescription([
        launch_ros.actions.Node(
            package='canbus_slave',
            executable='m2s_sender_can',
            output='screen'
            ),
        launch_ros.actions.Node(
            package='canbus_slave',
            executable='s2m_receiver_can',
            output='screen'
            ),
        ])

## to launch:
## ros2 launch canbus_slave canbus_slave.launch.py
