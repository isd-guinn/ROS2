# Copyright 2021 the Autoware Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Co-developed by Tier IV, Inc. and Apex.AI, Inc.

## to launch:
## ros2 launch ros2_socketcan socket_can_receiver.launch.py
## optional: specifying the variables
## ros2 launch ros2_socketcan socket_can_receiver.launch.py interface:=can1

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, EmitEvent,
                            RegisterEventHandler)
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessStart
from launch.events import matches_action
from launch.substitutions import LaunchConfiguration, TextSubstitution

from launch_ros.actions import LifecycleNode
# ROS-specific event handler: OnStateTransition 
from launch_ros.event_handlers import OnStateTransition
from launch_ros.events.lifecycle import ChangeState
from lifecycle_msgs.msg import Transition


def generate_launch_description():
    # first, we create a node that will receive CAN messages
    # this node is a LifecycleNode, which means it can be configured and activated
    socket_can_receiver_node = LifecycleNode(
        package='ros2_socketcan',
        executable='socket_can_receiver_node_exe',
        name='socket_can_receiver',
        namespace=TextSubstitution(text=''),
        # parameters are passed to the node from launch.xml
        parameters=[{
            # get the value of the 'interface' argument
            # using the LaunchConfiguration substitution
            # then pass it to the node as a parameter called 'interface'
            'interface': LaunchConfiguration('interface'), 
            'enable_can_fd': LaunchConfiguration('enable_can_fd'), 
            'interval_sec':
            LaunchConfiguration('interval_sec'), 
            'filters': LaunchConfiguration('filters'),
            'use_bus_time': LaunchConfiguration('use_bus_time'),
        }],
        remappings=[('from_can_bus', LaunchConfiguration('from_can_bus_topic'))], 
        output='screen'
        )

    # after create, it's in the 'unconfigured' state
    # then, we create an event handler that will configure the node when it starts
    
    # RegisterEventHandler is a way to register an event handler with the launch system, 
    # which will be triggered when the event occurs.
    socket_can_receiver_configure_event_handler = RegisterEventHandler(
        # define this event handler to be triggered when the node starts
        event_handler=OnProcessStart(
            target_action=socket_can_receiver_node, # when this node starts, below event will be triggered
            on_start=[
                EmitEvent(
                    event=ChangeState(
                        # matcher: to ensure the event is triggered by the correct node
                        lifecycle_node_matcher=matches_action(socket_can_receiver_node), 
                        # transit the node from 'unconfigured' to 'inactive' state via 'configure' transition
                        transition_id=Transition.TRANSITION_CONFIGURE, 
                        # The node’s onConfigure callback will then be called to allow the node to load its
                        # configuration from the parameter server and perform any other setup tasks.
                    ),
                ),
            ],
        ),
        # only run this event handler if the 'auto_configure' argument is set to true
        condition=IfCondition(LaunchConfiguration('auto_configure')),
    )

    # finally, we create an event handler that will activate the node when it is configured
    socket_can_receiver_activate_event_handler = RegisterEventHandler(
        # define to be triggered when the node *finish* transitions -- from 'configuring' to 'inactive'
        event_handler=OnStateTransition(
            target_lifecycle_node=socket_can_receiver_node,
            start_state='configuring',
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=matches_action(socket_can_receiver_node),
                        # activate the node
                        transition_id=Transition.TRANSITION_ACTIVATE,
                    ),
                ),
            ],
        ),
        # only run this event handler if the 'auto_activate' argument is set to true
        condition=IfCondition(LaunchConfiguration('auto_activate')),
    )

    # return the launch description
    return LaunchDescription([
        # DeclareLaunchArgument: expecting these arguments to be passed to the launch file
        # if not passed, default values will be used
        DeclareLaunchArgument('interface', default_value='can0'),
        DeclareLaunchArgument('enable_can_fd', default_value='false'),
        DeclareLaunchArgument('interval_sec', default_value='0.01'),
        DeclareLaunchArgument('use_bus_time', default_value='false'), # now using default
        # filter is a string that specifies which CAN messages to accept
        DeclareLaunchArgument('filters', default_value='0:0', # now using default
                              description='Comma separated filters can be specified for each given'
                                          ' CAN interface.\n'
                                          '\t<can_id>:<can_mask>\n'
                                          '\t\t(matches when <received_can_id> & mask == can_id & '
                                          'mask)\n'
                                          '\t<can_id>~<can_mask>\n'
                                          '\t\t(matches when <received_can_id> & mask != can_id & '
                                          'mask)\n'
                                          '\t#<error_mask>\n'
                                          '\t\t(set error frame filter, see include/linux/can/'
                                          'error.h)\n'
                                          '\t[j|J]\n'
                                          '\t\t(join the given CAN filters - logical AND '
                                          'semantic)\n\n'
                                          '\tCAN IDs, masks and data content are given and '
                                          'expected in hexadecimal values. When can_id and '
                                          'can_mask are both 8 digits, they are assumed to '
                                          "be 29 bit EFF. '0:0' default filter will accept "
                                          'all data frames.\n'
                                          '\tFor more information about syntax check: '
                                          'https://manpages.ubuntu.com/manpages/jammy/'
                                          'man1/candump.1.html'),
        DeclareLaunchArgument('auto_configure', default_value='true'), # now using default
        DeclareLaunchArgument('auto_activate', default_value='true'), # now using default
        DeclareLaunchArgument('from_can_bus_topic', default_value='from_can_bus'), 
        socket_can_receiver_node,
        socket_can_receiver_configure_event_handler,
        socket_can_receiver_activate_event_handler,
    ])
