#!/usr/bin/env python3
import rclpy
from rclpy.node import Node

import std_msgs.msg
from std_msgs.msg import UInt8, Header
import builtin_interfaces.msg
from builtin_interfaces.msg import Time
import imu_process.msg
from imu_process.msg import Position
import uart_slave.msg
from uart_slave.msg import FocAngle

from nav_algo import *

# global constants
STOP = 0
FORWARD = 1
BACKWARD = 2
ANTICLOCKWISE = 3
CLOCKWISE = 4
TIMER_PERIOD_IN_SECOND = 0.01 # seconds

def calculate_distance(dist, v_x, reached, dt):
    print("\nCalculating distance.....")

    dist += v_x * dt # seconds, dummy value
    print("dist: ", dist)
    dt = 0.0 # reset the time counter
    print("self.dt is reset: ", dt)
    if dist >= 30:
        # reached 30cm, reset the distance counter
        reached = True
        dist = 0.0
        print("dist is reset to: ", dist)
    print("reached: ", reached)
    return dist, reached, dt

class NavAlgo(Node):

    def __init__(self):
        super().__init__('nav_algo') # initialize the node
        
        self.nav_sub_pos_local_ = self.create_subscription(imu_process.msg.Position, 'Imu_local', self.position_callback, 10)
        # self.nav_sub_focangle_ = self.create_subscription(uart_slave.msg.FocAngle, 'Motor_voltage', self.focangle_callback, 10)
        self.nav_pub_action_ = self.create_publisher(std_msgs.msg.UInt8, '/Robot_action', 10)
        
        self.timer_ = self.create_timer(TIMER_PERIOD_IN_SECOND, self.timer_callback)
        
        # initialize the instance variables
        self.vel_x = 0.0
        self.vel_y = 0.0
        self.is_moving = False
        self.distance_counter = 0.0
        self.reach_distance = False
        self.foc_left = 0.0
        self.foc_right = 0.0
        self.dt = 0.0 # for keeping time for integration
        
    def position_callback(self, msg):
        print("\nposition_callback")
        print("vel_x: ", msg.vel_x)
        print("vel_y: ", msg.vel_y)
        self.vel_x = round(msg.vel_x, 4)
        self.vel_y = round(msg.vel_y, 4)
        print("self.vel_x: ", self.vel_x)
        print("self.vel_y: ", self.vel_y)
        
        # calculate the distance
        self.distance_counter, self.reach_distance, self.dt = calculate_distance(self.distance_counter, self.vel_x, self.reach_distance, self.dt)
        
        # threshold for determining if the robot is moving
        threshold = 0.1
        
        if self.vel_x > threshold:
            self.is_moving = True
            # run the navigation algorithm
            action = run_nav_algo(self.foc_left, self.foc_right, self.is_moving, self.reach_distance)
            print("action: ", action)
            # publish the action
            action_msg = std_msgs.msg.UInt8()
            action_msg.data = action
            print("action_msg.data: ", action_msg.data)
            self.nav_pub_action_.publish(action_msg)
        
        # reset the boolean
        self.is_moving = False
        self.reach_distance = False
        
    # def focangle_callback(self, msg):
    #     print("\nfocangle_callback")
    #     print("FOC left: ", msg.left)
    #     print("FOC right: ", msg.right)
    #     self.foc_left = precision(msg.left, 10)
    #     self.foc_right = precision(msg.right, 10)
    
    def timer_callback(self):
        # print("\n 0.01 second passed")
        self.dt += 0.01
        # print("self.dt: ", self.dt)

def main(arg=None):
    rclpy.init(args=arg)
    nav_algo = NavAlgo() # create the node
    rclpy.spin(nav_algo)
    rclpy.shutdown() 

if __name__ == '__main__':
    main()