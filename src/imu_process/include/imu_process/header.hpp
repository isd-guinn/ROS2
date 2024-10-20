#ifndef HEADER_HPP
#define HEADER_HPP

#include <stdint.h>
#include <Eigen/Dense>

#define DEG_TO_RAD  (0.01745329)

typedef struct
{
    float pos_x = 0.0f; // in local frame
    float pos_y = 0.0f; // in local frame
    float pos_z = 0.0f; // in local frame
    float vel_x;
    float vel_y;
    float vel_z;
    float acc_x_current;
    float acc_y_current;
    float acc_z_current;
    float acc_x_previous;
    float acc_y_previous;
    float acc_z_previous;
} position_t;

float precision(float value, int precision);

/* Output: position-related data in local frame */
void dead_reckon(position_t *data, const sensor_msgs::msg::Imu::SharedPtr *msg);

// Function to calculate rotation matrix from Euler angles using radians
Eigen::Matrix3f eulerToRotationMatrix(float roll, float pitch, float yaw);

void getGlobalPosition(float roll, float pitch, float yaw, float ax, float ay, float az, 
                    float dt, Eigen::Vector3f& velocity, Eigen::Vector3f& position);

#endif