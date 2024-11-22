##launch file
from launch import LaunchDescription
import launch_ros.actions 

def generate_launch_description():
    return LaunchDescription([
        launch_ros.actions.Node(
            package='canbus_test',
            executable='publisher',
            output='screen'
            ),
        ])

## to launch:
## ros2 launch canbus_test canbus_test.launch.py