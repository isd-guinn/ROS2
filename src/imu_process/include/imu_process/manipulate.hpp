#ifndef HEADER_HPP
#define HEADER_HPP

#include <stdint.h>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

#define DEG_TO_RAD  (0.01745329)

typedef struct
{
    Eigen::Vector3f pos_prev{{0.0, 0.0, 0.0}};
    Eigen::Vector3f vel_prev{{0.0, 0.0, 0.0}};
    Eigen::Vector3f acc_prev{{0.0, 0.0, 0.0}};
    Eigen::Vector3f pos_predict{{0.0, 0.0, 0.0}};
    Eigen::Vector3f vel_predict{{0.0, 0.0, 0.0}};
    Eigen::Vector3f acc_predict{{0.0, 0.0, 0.0}};
    Eigen::Vector3f acc_measured{{0.0, 0.0, 0.0}};
    Eigen::Vector3f vel_final{{0.0, 0.0, 0.0}};
    Eigen::Vector3f pos_final{{0.0, 0.0, 0.0}};
    Eigen::Vector3f acc_final{{0.0, 0.0, 0.0}};

    Eigen::Quaternionf quat = Eigen::Quaternionf(0.0, 0.0, 0.0, 0.0); // w, x, y, z

    // for testing the IEKF
    Eigen::Matrix<float, 3, 1> acc{{0.0, 0.0, 0.0}}; // acceleration in m/s^2
    Eigen::Matrix<float, 4, 1> quaternion{{0.0, 0.0, 0.0, 0.0}}; // quaternion (w,x,y,z)

    float angle_z = 0.0f; // z-axis
    float angVel_z_current = 0.0f;
    float angVel_z_previous = 0.0f;
} position_t;

/* --------- FOR IEKF ------------ */
class IEKF_DeadReckoning {
private:
    // State vector: [position (3), velocity (3), orientation (quaternion) (4)]
    Eigen::Matrix<float, 10, 1> x;
    // Covariance matrix
    Eigen::Matrix<float, 9, 9> P;
    // Process noise
    Eigen::Matrix<float, 9, 9> Q;
    // Measurement noise
    Eigen::Matrix<float, 7, 7> R;  // Changed to 7x7 for acc (3) + quaternion (4)
    // Gravity vector
    Eigen::Matrix<float, 3, 1> gravity;
    // Time step
    float dt;
    // Motion detection thresholds
    float acc_threshold;
    int static_counter;
    bool is_static;
    bool constant_velocity;

    float vel_threshold;    // For detecting complete stillness
    int static_window;      // Number of samples to confirm state
    std::vector<Eigen::Matrix<float, 3, 1>> velocity_history;  // For checking constant velocity

    // Convert quaternion to rotation matrix
    Eigen::Matrix<float, 3, 3> quatToRotMatrix(const Eigen::Matrix<float, 4, 1>& q) {
        Eigen::Matrix<float, 3, 3> R;
        float qw = q(0), qx = q(1), qy = q(2), qz = q(3);
        
        R << 1-2*(qy*qy+qz*qz), 2*(qx*qy-qw*qz), 2*(qx*qz+qw*qy),
             2*(qx*qy+qw*qz), 1-2*(qx*qx+qz*qz), 2*(qy*qz-qw*qx),
             2*(qx*qz-qw*qy), 2*(qy*qz+qw*qx), 1-2*(qx*qx+qy*qy);
        
        return R;
    }

public:
    // init
    IEKF_DeadReckoning(float time_step = 0.01f) 
        : dt(time_step), static_counter(0), is_static(false), constant_velocity(false) {
        
        // Initialize state vector
        x.setZero();
        x(6) = 1.0f; // Quaternion w component

        // Initialize covariance matrix
        P = Eigen::Matrix<float, 9, 9>::Identity() * 0.01f;

        // Process noise
        Q = Eigen::Matrix<float, 9, 9>::Identity();
        Q.block<3,3>(0,0) *= 0.01f; // position noise
        Q.block<3,3>(3,3) *= 0.01f; // velocity noise
        Q.block<3,3>(6,6) *= 0.01f; // orientation noise

        // Measurement noise
        R = Eigen::Matrix<float, 7, 7>::Identity();
        R.block<3,3>(0,0) *= 0.1f;  // accelerometer noise
        R.block<4,4>(3,3) *= 0.01f; // quaternion noise

        // Gravity vector
        gravity << 0.0f, 0.0f, 9.81f; // gravity is positive here (to be debug)

        // Motion detection thresholds
        acc_threshold = 0.05f;    // m/s^2
        vel_threshold = 0.05f;   // m/s
        static_window = 50;      // 0.5s at 100Hz
        velocity_history.reserve(static_window);
    }

    // predict stage: DOES NOT involve measurement
    void predict() {
        // Extract current state
        Eigen::Matrix<float, 3, 1> position = x.segment<3>(0);
        Eigen::Matrix<float, 3, 1> velocity = x.segment<3>(3);
        Eigen::Matrix<float, 4, 1> orientation = x.segment<4>(6);

        // Update position and velocity
        Eigen::Matrix<float, 3, 3> R = quatToRotMatrix(orientation);
        position += velocity * dt + 0.5f * R * gravity * dt * dt;
        velocity += R * gravity * dt;

        // Update state vector
        x.segment<3>(0) = position;
        x.segment<3>(3) = velocity;
        // Quaternion remains unchanged in prediction step

        // Compute Jacobians
        Eigen::Matrix<float, 9, 9> F = Eigen::Matrix<float, 9, 9>::Identity();
        F.block<3,3>(0,3) = Eigen::Matrix<float, 3, 3>::Identity() * dt;

        // Update covariance
        P = F * P * F.transpose() + Q;
    }

    // update stage: measurement is used
    void update(const Eigen::Matrix<float, 3, 1>& acc, const Eigen::Matrix<float, 4, 1>& quat) {
        // Check if robot is static or moving with constant velocity
        detectMotionState(acc);

        Eigen::Matrix<float, 3, 1> position = x.segment<3>(0);
        Eigen::Matrix<float, 3, 1> velocity = x.segment<3>(3);
        Eigen::Matrix<float, 4, 1> orientation = x.segment<4>(6);
        Eigen::Matrix<float, 3, 3> R = quatToRotMatrix(orientation);

        // Measurement vector
        Eigen::Matrix<float, 7, 1> z;
        z << acc, quat;

        // Expected measurement
        Eigen::Matrix<float, 7, 1> h;
        h << R.transpose() * gravity, orientation;

        // Innovation -- measurement - expected
        Eigen::Matrix<float, 7, 1> innovation = z - h;

        // Measurement Jacobian
        Eigen::Matrix<float, 7, 9> H = Eigen::Matrix<float, 7, 9>::Zero();
        H.block<3,3>(0,6) = Eigen::Matrix<float, 3, 3>::Identity();
        H.block<4,3>(3,6) = Eigen::Matrix<float, 4, 3>::Identity();

        // Kalman gain
        Eigen::Matrix<float, 7, 7> S = H * P * H.transpose() + R;
        Eigen::Matrix<float, 9, 7> K = P * H.transpose() * S.inverse();

        // Update state and covariance
        Eigen::Matrix<float, 9, 1> delta = K * innovation;
        x.segment<3>(0) += delta.segment<3>(0); // position
        x.segment<3>(3) += delta.segment<3>(3); // velocity
        x.segment<4>(6) = quat;  // Direct quaternion update from measurement

        // Update covariance
        P = (Eigen::Matrix<float, 9, 9>::Identity() - K * H) * P;
    }

    // decide whether the robot is moving in constant velocity OR is stopped
    void detectMotionState(const Eigen::Matrix<float, 3, 1>& acc) {
        Eigen::Matrix<float, 4, 1> orientation = x.segment<4>(6);
        Eigen::Matrix<float, 3, 3> R = quatToRotMatrix(orientation);
        Eigen::Matrix<float, 3, 1> acc_without_gravity = acc - R.transpose() * gravity;
        Eigen::Matrix<float, 3, 1> current_velocity = x.segment<3>(3);
        
        velocity_history.push_back(current_velocity); // push to the back
        if (velocity_history.size() > static_window) {
            velocity_history.erase(velocity_history.begin()); // pop from the front
        }

        // Case 1: Complete stillness detection
        if (acc_without_gravity.norm() < acc_threshold && 
            current_velocity.norm() < vel_threshold) {
            static_counter++;
            if (static_counter > static_window) {
                is_static = true;
                constant_velocity = false;
                x.segment<3>(3).setZero(); // Reset current velocity
            }
        }
        // Case 2: Constant velocity detection
        else if (acc_without_gravity.norm() < acc_threshold && 
                 velocity_history.size() == static_window) {
            // Calculate velocity variation over the window
            Eigen::Matrix<float, 3, 1> avg_velocity = Eigen::Matrix<float, 3, 1>::Zero();
            for (const auto& vel : velocity_history) {
                avg_velocity += vel;
            }
            avg_velocity /= static_window;

            // Check if velocity has been consistent
            bool velocity_consistent = true;
            for (const auto& vel : velocity_history) {
                if ((vel - avg_velocity).norm() > vel_threshold) {
                    velocity_consistent = false;
                    break;
                }
            }
            if (velocity_consistent && avg_velocity.norm() > vel_threshold) {
                is_static = false;
                constant_velocity = true;
            } else {
                is_static = false;
                constant_velocity = false;
            }
        }
        // Case 3: Accelerating/Decelerating
        else {
            static_counter = 0;
            is_static = false;
            constant_velocity = false;
        }
    }
    bool isStatic() const { return is_static; }
    bool isConstantVelocity() const { return constant_velocity; }

    Eigen::Matrix<float, 3, 1> getPosition() const { return x.segment<3>(0); }
    Eigen::Matrix<float, 3, 1> getVelocity() const { return x.segment<3>(3); }
    Eigen::Matrix<float, 4, 1> getOrientation() const { return x.segment<4>(6); }
};

/* --------- OTHERS ------------ */
float precision(float value, int precision);

/* --------- FOR LOCAL FRAMES ------------ */

/* Output: position-related data in local frame, return elapse time */
double dead_reckon(position_t *data, double &last_update_time);
/* Output: Current Angle in local frame */
void update_angle(position_t *data, double elapsed_time);

/* --------- FOR FRAME TRANSFORMATION ------------ */

Eigen::Array3f distributeGravity (Eigen::Matrix3f R);
Eigen::Matrix3f quaternionToRotationMatrix(Eigen::Quaternionf quat);

/*  To calculate rotational matrix.
*   Input:  (float) roll, pitch, raw -- euler angles in radian
*   Output: (Eigen::Matrix3f) -- rotational matrix
*/
// Eigen::Matrix3f eulerToRotationMatrix(float roll, float pitch, float yaw);

// void getGlobalPosition(float roll, float pitch, float yaw, float accel_local, Eigen::Vector3f& velocity, Eigen::Vector3f& position);

#endif