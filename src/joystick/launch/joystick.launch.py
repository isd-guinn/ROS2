##launch file
from launch import LaunchDescription
import launch_ros.actions 

def generate_launch_description():
    return LaunchDescription([
        launch_ros.actions.Node(
            package='joystick',
            executable='joystick',
            output='screen'
            ),
        ])

## to launch:
## ros2 launch joystick joystick.launch.py