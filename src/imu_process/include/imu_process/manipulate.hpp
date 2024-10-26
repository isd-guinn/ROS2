#ifndef HEADER_HPP
#define HEADER_HPP

#include <stdint.h>
// #include <Eigen/Dense>

#define DEG_TO_RAD  (0.01745329)

typedef struct {
    float x;
    float y;
    float z;
} vector_t;

typedef struct
{
    vector_t pos = {0.0f, 0.0f, 0.0f};
    vector_t vel;
    vector_t acc_current;
    vector_t acc_prev;
    // float pos_x = 0.0f; 
    // float pos_y = 0.0f; 
    // float pos_z = 0.0f; 
    // float vel_x;
    // float vel_y;
    // float vel_z;
    // float acc_x_current;
    // float acc_y_current;
    // float acc_z_current;
    // float acc_x_previous;
    // float acc_y_previous;
    // float acc_z_previous;

    float angle_z = 0.0f; // z-axis
    float angVel_z_current;
    float angVel_z_previous;
} position_t;

float precision(float value, int precision);

/* --------- FOR LOCAL FRAMES ------------ */

/* Output: position-related data in local frame */
void dead_reckon(position_t *data);
/* Output: Current Angle in local frame */
void update_angle(position_t *data);

/* --------- FOR FRAME TRANSFORMATION ------------ */

/*  To calculate rotational matrix.
*   Input:  (float) roll, pitch, raw -- euler angles in radian
*   Output: (Eigen::Matrix3f) -- rotational matrix
*/
// Eigen::Matrix3f eulerToRotationMatrix(float roll, float pitch, float yaw);

// void getGlobalPosition(float roll, float pitch, float yaw, float accel_local, Eigen::Vector3f& velocity, Eigen::Vector3f& position);

#endif