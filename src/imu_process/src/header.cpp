#include "imu_process/header.hpp"
#include <sensor_msgs/msg/imu.hpp>
#include <iostream>

#include <cmath>
#include <Eigen/Dense>

using namespace std;

float precision( float value, int precision )
{
    float n = std::pow(10.0f, precision);
    return std::round(value * n) / n ;
}

void dead_reckon(position_t *data, const sensor_msgs::msg::Imu::SharedPtr *msg)
{
    float alpha = 0.9;
    double dt = 0.1; // 100ms time step (assuming constant for simplicity)

    // get acceleration from IMU data
    data->acc_x_current = precision(msg->linear_acceleration.x, 18);
    data->acc_y_current = precision(msg->linear_acceleration.y, 18);

    // Low-pass filter the acceleration data
    data->acc_x_current = alpha * data->acc_x_current + (1 - alpha) * data->acc_x_previous;
    data->acc_y_current = alpha * data->acc_y_current + (1 - alpha) * data->acc_y_previous;

    // Integrate to get velocity & position
    data->vel_x += data->acc_x_current * dt;
    data->vel_y += data->acc_y_current * dt;
    data->pos_x += data->vel_x * dt;
    data->pos_y += data->vel_y * dt;

    // save current acceleration for next iteration
    data->acc_x_previous = data->acc_x_current;
    data->acc_y_previous = data->acc_y_current;
}

// roll, pitch, yaw in radians
Eigen::Matrix3f eulerToRotationMatrix(float roll, float pitch, float yaw) {
    // Z-axis rotation matrix (yaw)
    Eigen::Matrix3f Rz;
    Rz << cos(yaw), -sin(yaw), 0,
          sin(yaw), cos(yaw), 0,
          0, 0, 1;

    // X-axis rotation matrix (pitch)
    Eigen::Matrix3f Rx;
    Rx << 1, 0, 0,
          0, cos(roll), -sin(roll),
          0, sin(roll), cos(roll);

    // Y-axis rotation matrix (roll)
    Eigen::Matrix3f Ry;
    Rx << cos(pitch), 0, sin(pitch),
          0, 1, 0,
          -sin(pitch), 0, cos(pitch);

    // Final rotation matrix: R = Ry * Rx * Rz (z->x->y)
    return Ry * Rx * Rz;
}

void getGlobalPosition(float roll, float pitch, float yaw, float ax, float ay, float az,
                       float dt, Eigen::Vector3f &velocity, Eigen::Vector3f &position)
{
    double dt = 0.1;

    // Convert Euler angles to rotation matrix (z->x->y)
    Eigen::Matrix3f rotationMatrix = eulerToRotationMatrix(roll, pitch, yaw);

    /* Convert acceleration to global frame 
    * accel_local = [ax, ay, az] from imu_processed
    */
    Eigen::Vector3f accel_global = rotationMatrix * accel_local;

    accel_global[2] -= 9.81; // subtract gravity


    // Integrate to get velocity & position (global frame)
    velocity += accel_global * dt;
    position += velocity * dt;
}