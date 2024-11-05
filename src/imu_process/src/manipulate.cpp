#include "imu_process/manipulate.hpp"
#include <sensor_msgs/msg/imu.hpp>
#include <iostream>

#include <cmath>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>
// #include <Eigen/Dense>
#include "rclcpp/rclcpp.hpp"

#define GRAVITY     (9.81)
#define DEG_TO_RAD  (0.01745329)

using namespace std;

float precision( float value, int precision )
{
    float n = std::pow(10.0f, precision);
    return std::round(value * n) / n ;
}

double dead_reckon(position_t *data, double &last_update_time)
{
    // double dt = 0.1; // 100ms time step (assuming constant for simplicity)
    float alpha = 0.1; 
    double current_time = double_t(rclcpp::Clock().now().seconds());
    // std::cout << "Current time: " << current_time << std::endl;
    // std::cout << "Last update time: " << last_update_time << std::endl;
    double elapsed_time = current_time - last_update_time;
    // std::cout << "Elapsed time: " << elapsed_time << std::endl;

    if (elapsed_time == 0) {
        std::cout << "Elapsed time is 0" << std::endl;
        return 0.0; // NULL
    }

    if (elapsed_time > 0.1) {
        std::cout << "Elapsed time is too large" << std::endl;
        last_update_time = double_t(rclcpp::Clock().now().seconds());
        return 0.0; // NULL
    }

    // estimate
    data->vel_predict = data->vel_prev;
    data->acc_predict.setZero();
    data->pos_predict = data->pos_prev + data->vel_prev * elapsed_time;
    std::cout << "Predicted Position: " << data->pos_predict.transpose() << std::endl;
    std::cout << "Predicted Velocity: " << data->vel_predict.transpose() << std::endl;

    data->acc_final = (1-alpha) * data->acc_predict + alpha * data->acc_measured;
    if (abs(data->acc_final(0)) < 0.05) data->acc_final(0) = 0;
    if (abs(data->acc_final(1)) < 0.05) data->acc_final(1) = 0;
    data->vel_final = data->vel_prev + alpha * data->acc_final * elapsed_time;
    data->pos_final = data->pos_prev + data->vel_prev * elapsed_time + 0.5 * alpha * data->acc_final * elapsed_time * elapsed_time;

    data->acc_prev = data->acc_final;
    data->vel_prev = data->vel_final;
    data->pos_prev = data->pos_final;

    last_update_time = current_time;
    return elapsed_time;
}

void update_angle(position_t *data, double elapsed_time){
    if (elapsed_time == NULL) {
        std::cout << "Elapsed time is 0" << std::endl;
        return;
    }
    
    // double dt = 0.1;
    // data->angle_z += data->angVel_z_current * dt;

    // low-pass filter for angular velocity
    float alpha = 0.2;
    float temp = data->angle_z / DEG_TO_RAD;
    std::cout << "previous Angle (degree): " << temp << std::endl;
    float temp2 = data->angVel_z_previous / DEG_TO_RAD;
    std::cout << "previous AngVel: " << temp2 << std::endl;
    float temp3 = data->angVel_z_current / DEG_TO_RAD;
    std::cout << "measured AngVel: " << temp3 << std::endl;
    data->angVel_z_current = (1 - alpha) * data->angVel_z_previous + alpha * data->angVel_z_current;
    if (abs(data->angVel_z_current) < 0.003) data->angVel_z_current = 0;
    float temp4 = data->angVel_z_current / DEG_TO_RAD;
    std::cout << "filtered AngVel: " << temp4 << std::endl;

    // integrate to get angle
    data->angle_z += data->angVel_z_current * elapsed_time;
    float temp5 = data->angle_z / DEG_TO_RAD;
    std::cout << "Updated Angle: " << temp5 << std::endl;

    // save current acceleration for next iteration
    data->angVel_z_previous = data->angVel_z_current;
}

Eigen::Matrix3f quaternionToRotationMatrix(Eigen::Quaternionf quat) 
{
    Eigen::Matrix3f R;
    R.setZero();
    quat.normalize();
    R = quat.toRotationMatrix();

    // float w = quat(0);
    // float x = quat(1);
    // float y = quat(2);
    // float z = quat(3);

    // R(0, 0) = 1- 2*(y*y + z*z);
    // R(0, 1) = 2*(x*y - w*z);
    // R(0, 2) = 2*(x*z + w*y);
    // R(1, 0) = 2*(x*y + w*z);
    // R(1, 1) = 1- 2*(x*x + z*z);
    // R(1, 2) = 2*(y*z - w*x);
    // R(2, 0) = 2*(x*z - w*y);
    // R(2, 1) = 2*(y*z + w*x);
    // R(2, 2) = 1- 2*(x*x + y*y);
    return R;
}

// Distribute gravity in local frame
Eigen::Array3f distributeGravity (Eigen::Matrix3f R) {
    Eigen::Vector3f gravity;
    gravity = Eigen::Vector3f::UnitZ() * GRAVITY;
    // std::cout << "Gravity: " << gravity.transpose() << std::endl;
    Eigen::Array3f gravity_local = R * gravity;
    return gravity_local;
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