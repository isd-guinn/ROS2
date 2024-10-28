#include "imu_process/manipulate.hpp"
#include <sensor_msgs/msg/imu.hpp>
#include <iostream>

#include <cmath>
// #include <Eigen/Dense>
#include "rclcpp/rclcpp.hpp"

using namespace std;

float precision( float value, int precision )
{
    float n = std::pow(10.0f, precision);
    return std::round(value * n) / n ;
}

void dead_reckon(position_t *data, double &last_update_time)
{
    // double dt = 0.1; // 100ms time step (assuming constant for simplicity)
    float alpha = 0.1; 
    double current_time = double_t(rclcpp::Clock().now().seconds());
    std::cout << "Current time: " << current_time << std::endl;
    // std::cout << "Last update time: " << last_update_time << std::endl;
    double elapsed_time = current_time - last_update_time;
    std::cout << "Elapsed time: " << elapsed_time << std::endl;

    if (elapsed_time == 0) {
        std::cout << "Elapsed time is 0" << std::endl;
        return;
    }

    if (elapsed_time > 0.1) {
        std::cout << "Elapsed time is too large" << std::endl;
        last_update_time = double_t(rclcpp::Clock().now().seconds());
        return;
    }

    // estimate
    data->vel_predict.x = data->vel_prev.x;
    data->vel_predict.y = data->vel_prev.y;
    data->acc_predict.x = 0;
    data->acc_predict.y = 0;
    data->pos_predict.x = data->pos_prev.x + data->vel_prev.x * elapsed_time;
    data->pos_predict.y = data->pos_prev.y + data->vel_prev.y * elapsed_time;
    std::cout << "Predicted Position: " << data->pos_predict.x << " " << data->pos_predict.y << std::endl;
    std::cout << "Predicted Velocity: " << data->vel_predict.x << " " << data->vel_predict.y << std::endl;

    data->acc_final.x = (1-alpha) * data->acc_predict.x + alpha * data->acc_measured.x;
    data->acc_final.y = (1-alpha) * data->acc_predict.y + alpha * data->acc_measured.y;
    if (abs(data->acc_final.x) < 0.05) data->acc_final.x = 0;
    if (abs(data->acc_final.y) < 0.05) data->acc_final.y = 0;
    data->vel_final.x = data->vel_prev.x + alpha * data->acc_final.x * elapsed_time;
    data->vel_final.y = data->vel_prev.y + alpha * data->acc_final.y * elapsed_time;
    data->pos_final.x = data->pos_prev.x + data->vel_prev.x * elapsed_time + 0.5 * alpha * data->acc_final.x * elapsed_time * elapsed_time;
    data->pos_final.y = data->pos_prev.y + data->vel_prev.y * elapsed_time + 0.5 * alpha * data->acc_final.y * elapsed_time * elapsed_time;

    data->acc_prev.x = data->acc_final.x;
    data->acc_prev.y = data->acc_final.y;
    data->vel_prev.x = data->vel_final.x;
    data->vel_prev.y = data->vel_final.y;
    data->pos_prev.x = data->pos_final.x;
    data->pos_prev.y = data->pos_final.y;

    last_update_time = current_time;
}

void update_angle(position_t *data){
    // double dt = 0.1;
    // data->angle_z += data->angVel_z_current * dt;

    // low-pass filter for angular velocity
    float alpha = 0.1;
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