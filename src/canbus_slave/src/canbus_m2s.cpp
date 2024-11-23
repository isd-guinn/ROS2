#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int8.hpp"
#include "can_msgs/msg/frame.hpp"
// #include "ros2_socketcan_msgs/msg/fd_frame.hpp"
#include "canbus_slave/msg/foc_angle.hpp"

#define SMALL_ENDIAN
// #define BIG_ENDIAN

#include "canbus_slave/MasterCanProtocol.hpp"

using namespace std;

class CanbusSender : public rclcpp::Node
{
public:
    CanbusSender()
        : Node("Canbus_decoder")
    {
        action_sub_ = this->create_subscription<std_msgs::msg::UInt8>(
            "Robot_action", 10, std::bind(&CanbusSender::action_callback, this, std::placeholders::_1));
        motorvolt_sub_ = this->create_subscription<canbus_slave::msg::FocAngle>(
            "Motor_volt", 10, std::bind(&CanbusSender::motorvolt_callback, this, std::placeholders::_1));
        
        can_pub_ = this->create_publisher<can_msgs::msg::Frame>(
            "/to_can_bus", 10);
    }

private:
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr action_sub_;
    rclcpp::Subscription<canbus_slave::msg::FocAngle>::SharedPtr motorvolt_sub_;
    rclcpp::Publisher<can_msgs::msg::Frame>::SharedPtr can_pub_;

    void action_callback(const std_msgs::msg::UInt8::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Action received: '%d'", msg->data);
        std::cout << "Action received: " << msg->data << std::endl;

        can_msgs::msg::Frame can_msg;
        can_msg.id = ID_ACTION;
        can_msg.dlc = CAN_FRAME_DLC;
        can_msg.is_extended = false;
        can_msg.is_rtr = false;
        can_msg.is_error = false;

        // encode the data
        can_msg.data[BYTE_POS_ACTION] = msg->data;
        for (int i = BYTE_POS_ACTION + 1; i < CAN_FRAME_DLC; i++){
            can_msg.data[i] = 0;
        }
        
        std::cout << "Action frame data: " << std::endl;
        for (int i = 0; i < CAN_FRAME_DLC; i++){
            std::cout << std::hex << (int)can_msg.data[i] << " ";
        }
        std::cout << std::endl;

        can_pub_->publish(can_msg);
        std::cout << "Action published\n" << std::endl;
    }

    void motorvolt_callback(const canbus_slave::msg::FocAngle::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "Motor voltage received: '%f', '%f'", msg->left, msg->right);
        std::cout << "Motor voltage received: " << msg->left << ", " << msg->right << std::endl;

        can_msgs::msg::Frame can_msg;
        can_msg.id = ID_MOTOR_VOLTAGE;
        can_msg.dlc = CAN_FRAME_DLC;
        can_msg.is_extended = false;
        can_msg.is_rtr = false;
        can_msg.is_error = false;

        // encode the data
        for (int i = BYTE_POS_MOTOR_VOLT_LEFT; i < BYTE_POS_MOTOR_VOLT_RIGHT; i++){
            can_msg.data[i] = EXTRACT_BYTE_FROM_4BYTE_VALUE(msg->left, i - BYTE_POS_MOTOR_VOLT_LEFT);
        }
        for (int i = BYTE_POS_MOTOR_VOLT_RIGHT; i < CAN_FRAME_DLC; i++){
            can_msg.data[i] = EXTRACT_BYTE_FROM_4BYTE_VALUE(msg->right, i - BYTE_POS_MOTOR_VOLT_RIGHT);
        }

        std::cout << "Motor voltage frame data: " << std::endl;
        for (int i = 0; i < CAN_FRAME_DLC; i++){
            std::cout << std::hex << (int)can_msg.data[i] << " ";
        }
        std::cout << std::endl;
        
        can_pub_->publish(can_msg);
        std::cout << "Motor voltage published\n" << std::endl;
    }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CanbusSender>());
  rclcpp::shutdown();
  return 0;
}