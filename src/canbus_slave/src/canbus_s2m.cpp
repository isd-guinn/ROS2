#include "rclcpp/rclcpp.hpp"
// #include "std_msgs/msg/header.hpp"
#include "can_msgs/msg/frame.hpp"
// #include "ros2_socketcan_msgs/msg/fd_frame.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "serial_imu/msg/euler_angle.hpp"
#include "uart_slave/msg/foc_angle.hpp"

#define SMALL_ENDIAN
// #define BIG_ENDIAN

#include "canbus_slave/MasterCanProtocol.hpp"
#include "canbus_slave/CanDecode.hpp"

using namespace std;
static raw_t raw; // struct for storing the raw data from CAN bus
static raw_imu_t history_imu;

class CanbusReceiver : public rclcpp::Node
{
public:
    CanbusReceiver()
        : Node("Canbus_decoder")
    {
        can_sub_ = this->create_subscription<can_msgs::msg::Frame>(
            "from_can_bus", 10, std::bind(&CanbusReceiver::canbus_callback, this, std::placeholders::_1));
        // can_fd_sub_ = this->create_subscription<ros2_socketcan_msgs::msg::FdFrame>(
        //     "from_can_bus_fd", 10, std::bind(&CanbusReceiver::canbus_fd_callback, this, std::placeholders::_1));
        
        imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>(
            "/Imu_data_can", 10);
        euler_pub_ = this->create_publisher<serial_imu::msg::EulerAngle>(
            "/Imu_euler_angle", 10);
        foc_pub_ = this->create_publisher<uart_slave::msg::FocAngle>(
            "/FOC_angle", 10);
    }

private:
    rclcpp::Subscription<can_msgs::msg::Frame>::SharedPtr can_sub_;
    // rclcpp::Subscription<ros2_socketcan_msgs::msg::FdFrame>::SharedPtr can_fd_sub_;

    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<serial_imu::msg::EulerAngle>::SharedPtr euler_pub_;
    rclcpp::Publisher<uart_slave::msg::FocAngle>::SharedPtr foc_pub_;

    void canbus_callback(const can_msgs::msg::Frame::SharedPtr msg)
    {
        std::cout << "\nmessage id: " << msg->id << std::endl;
        // decode & store
        if (can_decode(msg, &raw) == -1)
        {
            std::cout << "Error in decoding CAN message" << std::endl;
            return;
        }

        // publish based on the message id
        if (msg->id == ID_FOC){
            // publish FOC angle
            uart_slave::msg::FocAngle foc_msg;
            foc_msg.left = raw.foc.foc_left;
            foc_msg.right = raw.foc.foc_right;
            foc_pub_->publish(foc_msg);
            std::cout << "FOC angle published" << std::endl;
        }
        else{
            // publish IMU data
            if (msg->id == ID_IMU_ANG_XY || msg->id == ID_IMU_ANG_Z){
                // publish IMU angle data - euler angles
                serial_imu::msg::EulerAngle euler_msg;
                // load the previous data
                euler_msg.pitch_x = history_imu.ang_x;
                euler_msg.roll_y = history_imu.ang_y;
                euler_msg.yaw_z = history_imu.ang_z;
                // update the data
                switch (msg->id){
                    case ID_IMU_ANG_XY:
                        euler_msg.pitch_x = raw.imu.ang_x;
                        euler_msg.roll_y = raw.imu.ang_y;
                        break;
                    case ID_IMU_ANG_Z:
                        euler_msg.yaw_z = raw.imu.ang_z;
                        break;
                    default:
                        std::cout << "EULER ANGLE ID not found" << std::endl;
                        break;
                }
                euler_pub_->publish(euler_msg);
                std::cout << "Euler Angle published" << std::endl;
                // update the history data
                history_imu.ang_x = euler_msg.pitch_x;
                history_imu.ang_y = euler_msg.roll_y;
                history_imu.ang_z = euler_msg.yaw_z;
            }
            else{
                // publish IMU data
                sensor_msgs::msg::Imu imu_msg;
                imu_msg.header.stamp = rclcpp::Clock().now();
                imu_msg.header.frame_id = "base_link";
                // load the previous data
                imu_msg.linear_acceleration.x = history_imu.acc_x;
                imu_msg.linear_acceleration.y = history_imu.acc_y;
                imu_msg.linear_acceleration.z = history_imu.acc_z;
                imu_msg.angular_velocity.x = history_imu.angvel_x;
                imu_msg.angular_velocity.y = history_imu.angvel_y;
                imu_msg.angular_velocity.z = history_imu.angvel_z;
                imu_msg.orientation.x = history_imu.quat_x;
                imu_msg.orientation.y = history_imu.quat_y;
                imu_msg.orientation.z = history_imu.quat_z;
                imu_msg.orientation.w = history_imu.quat_w;
                switch (msg->id){
                    case ID_IMU_ACC_XY:
                        imu_msg.linear_acceleration.x = raw.imu.acc_x;
                        imu_msg.linear_acceleration.y = raw.imu.acc_y;
                        imu_pub_->publish(imu_msg);
                        std::cout << "IMU data published" << std::endl;
                        // update the history data
                        history_imu.acc_x = raw.imu.acc_x;
                        history_imu.acc_y = raw.imu.acc_y;
                        break;
                    case ID_IMU_ACC_Z:
                        imu_msg.linear_acceleration.z = raw.imu.acc_z;
                        imu_pub_->publish(imu_msg);
                        std::cout << "IMU data published" << std::endl;
                        // update the history data
                        history_imu.acc_z = raw.imu.acc_z;
                        break;
                    case ID_IMU_ANGVEL_XY:
                        imu_msg.angular_velocity.x = raw.imu.angvel_x;
                        imu_msg.angular_velocity.y = raw.imu.angvel_y;
                        imu_pub_->publish(imu_msg);
                        std::cout << "IMU data published" << std::endl;
                        // update the history data
                        history_imu.angvel_x = raw.imu.angvel_x;
                        history_imu.angvel_y = raw.imu.angvel_y;
                        break;
                    case ID_IMU_ANGVEL_Z:
                        imu_msg.angular_velocity.z = raw.imu.angvel_z;
                        imu_pub_->publish(imu_msg);
                        std::cout << "IMU data published" << std::endl;
                        // update the history data
                        history_imu.angvel_z = raw.imu.angvel_z;
                        break;
                    case ID_IMU_QUAT_XY:
                        imu_msg.orientation.x = raw.imu.quat_x;
                        imu_msg.orientation.y = raw.imu.quat_y;
                        imu_pub_->publish(imu_msg);
                        std::cout << "IMU data published" << std::endl;
                        // update the history data
                        history_imu.quat_x = raw.imu.quat_x;
                        history_imu.quat_y = raw.imu.quat_y;
                        break;
                    case ID_IMU_QUAT_ZW:
                        imu_msg.orientation.z = raw.imu.quat_z;
                        imu_msg.orientation.w = raw.imu.quat_w;
                        imu_pub_->publish(imu_msg);
                        std::cout << "IMU data published" << std::endl;
                        // update the history data
                        history_imu.quat_z = raw.imu.quat_z;
                        history_imu.quat_w = raw.imu.quat_w;
                        break;
                    default:
                        std::cout << "IMU DATA ID not found" << std::endl;
                        break;
                }
            }
        }
    }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CanbusReceiver>());
  rclcpp::shutdown();
  return 0;
}