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
    vector_t pos_prev = {0.0f, 0.0f, 0.0f};
    vector_t vel_prev = {0.0f, 0.0f, 0.0f};
    vector_t acc_prev = {0.0f, 0.0f, 0.0f};
    vector_t pos_predict = {0.0f, 0.0f, 0.0f};
    vector_t vel_predict = {0.0f, 0.0f, 0.0f};
    vector_t acc_predict = {0.0f, 0.0f, 0.0f};
    vector_t acc_measured = {0.0f, 0.0f, 0.0f};
    vector_t vel_final = {0.0f, 0.0f, 0.0f};
    vector_t pos_final = {0.0f, 0.0f, 0.0f};
    vector_t acc_final = {0.0f, 0.0f, 0.0f};

    float angle_z = 0.0f; // z-axis
    float angVel_z_current = 0.0f;
    float angVel_z_previous = 0.0f;
} position_t;

float precision(float value, int precision);

/* --------- FOR LOCAL FRAMES ------------ */

/* Output: position-related data in local frame */
void dead_reckon(position_t *data, double &last_update_time);
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