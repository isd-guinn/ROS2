#include "imu_process/manipulate.hpp"
#include <sensor_msgs/msg/imu.hpp>
#include <iostream>

#include <cmath>
// #include <Eigen/Dense>

using namespace std;

float precision( float value, int precision )
{
    float n = std::pow(10.0f, precision);
    return std::round(value * n) / n ;
}

void dead_reckon(position_t *data)
{
    double dt = 0.1; // 100ms time step (assuming constant for simplicity)
    float alpha = 9 * dt; // ALPHA INCREASE WITH INCREASE dt

    // Low-pass filter for acceleration
    data->acc_current.x = alpha * data->acc_current.x + (1 - alpha) * data->acc_prev.x;
    data->acc_current.y = alpha * data->acc_current.x + (1 - alpha) * data->acc_prev.y;

    // Simple Integrate for velocity & position
    data->vel.x += data->acc_current.x * dt;
    data->vel.y += data->acc_current.y * dt;
    data->pos.x += data->vel.x * dt;
    data->pos.y += data->vel.x * dt;

    // save current acceleration for next iteration
    data->acc_prev.x = data->acc_current.x;
    data->acc_prev.y = data->acc_current.y;
}

void update_angle(position_t *data){
    double dt = 0.1;
    // data->angle_z += data->angVel_z_current * dt;

    // low-pass filter for angular velocity
    float alpha = 9 * dt; // ALPHA INCREASE WITH INCREASE dt
    data->angVel_z_current = alpha * data->angVel_z_current + (1 - alpha) * data->angVel_z_previous;

    // save current acceleration for next iteration
    data->angVel_z_previous = data->angVel_z_current;
}

/* eulerToRotationMatrix (input: roll, pitch, yaw in radians)
Eigen::Matrix3f eulerToRotationMatrix(float roll, float pitch, float yaw) {
    // Z-axis rotation matrix (yaw)
    // Eigen::Matrix3f Rz;
    // Rz.setZero();
    // Rz << cos(yaw), -sin(yaw), 0,
    //       sin(yaw), cos(yaw), 0,
    //       0, 0, 1;

    // X-axis rotation matrix (pitch)
    // Eigen::Matrix3f Rx;
    // Rx.setZero();
    // Rx << 1, 0, 0,
    //       0, cos(roll), -sin(roll),
    //       0, sin(roll), cos(roll);

    // Y-axis rotation matrix (roll)
    Eigen::Matrix3f Ry;
    Ry.setZero();
    // Rx << cos(pitch), 0, sin(pitch),
    //       0, 1, 0,
    //       -sin(pitch), 0, cos(pitch);

    // Final rotation matrix: R = Ry * Rx * Rz (z->x->y)
    // return Ry * Rx * Rz;
    return Ry;
}
// */

/* getGlobalPosition
void getGlobalPosition(float roll, float pitch, float yaw, float accel_local, Eigen::Vector3f &velocity, Eigen::Vector3f &position)
{
    double dt = 0.1;

    // Convert Euler angles to rotation matrix (z->x->y)
    Eigen::Matrix3f rotationMatrix = eulerToRotationMatrix(roll, pitch, yaw);

    // accel_local = [ax, ay, az] from imu_processed
    Eigen::Vector3f accel_global = rotationMatrix * accel_local;

    accel_global[2] -= 9.81; // subtract gravity

    // Integrate to get velocity & position (global frame)
    velocity += accel_global * dt;
    position += velocity * dt;
}
// */