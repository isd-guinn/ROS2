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
        
        // imu_pub_ = this->create_publisher<???>(
        //     "/Imu_data_can", 10);
        // euler_pub_ = this->create_publisher<serial_imu::msg::EulerAngle>(
        //     "/Imu_euler_angle", 10);
        foc_pub_ = this->create_publisher<uart_slave::msg::FocAngle>(
            "/Foc_angle", 10);
    }

private:
    rclcpp::Subscription<can_msgs::msg::Frame>::SharedPtr can_sub_;
    // rclcpp::Subscription<ros2_socketcan_msgs::msg::FdFrame>::SharedPtr can_fd_sub_;

    // rclcpp::Publisher<???>::SharedPtr imu_pub_;
    // rclcpp::Publisher<serial_imu::msg::EulerAngle>::SharedPtr euler_pub_;
    rclcpp::Publisher<uart_slave::msg::FocAngle>::SharedPtr foc_pub_;

    void canbus_callback(const can_msgs::msg::Frame::SharedPtr msg)
    {
        // decode & store
        if (can_decode(msg, &raw) == -1)
        {
            std::cout << "Error in decoding CAN message" << std::endl;
            return;
        }

        // publish based on the message id
        if (msg->id == ID_FOC_ANGLE){
            // publish FOC angle
            uart_slave::msg::FocAngle foc_msg;
            foc_msg.header.stamp = msg->header.stamp;
            foc_msg.foc_left = raw.foc.foc_left;
            foc_msg.foc_right = raw.foc.foc_right;
            foc_pub_->publish(foc_msg);
            std::cout << "FOC angle published" << std::endl;
        }
        else{
            // publish IMU data
            std::cout << "IMU data published (placeholder only)" << std::endl;
        }
    }
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CanbusReceiver>());
  rclcpp::shutdown();
  return 0;
}