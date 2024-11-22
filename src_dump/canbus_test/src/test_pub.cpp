#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int8.hpp"
#include "uart_slave/msg/foc_angle.hpp"

using namespace std;

class TestPub : public rclcpp::Node
{
public:
    TestPub()
        : Node("Testing_Publisher")
    {        
        motorvolt_pub_ = this->create_publisher<uart_slave::msg::FocAngle>(
            "/Motor_voltage", 10);
        action_pub_ = this->create_publisher<std_msgs::msg::UInt8>(
            "/Robot_action", 10);
        timer_ = this->create_wall_timer(1000ms, std::bind(&TestPub::timer_callback, this));
    }

private:
    rclcpp::Publisher<uart_slave::msg::FocAngle>::SharedPtr motorvolt_pub_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr action_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    void timer_callback()
    {
        std::cout << "Publishing Motor Voltage" << std::endl;
        auto motorvolt = uart_slave::msg::FocAngle();
        // ask for keyboard input to be the left and the right data
        bool manual_input = true;
        if (manual_input)
        {
            std::cout << "Enter left and right motor voltage (float): ";
            std::cin >> motorvolt.left;
            std::cin >> motorvolt.right;
        }
        else {
            motorvolt.left = 8.0;
            motorvolt.right = 8.0;
        }
        motorvolt_pub_->publish(motorvolt);
        std::cout << "Published Motor Voltage" << std::endl;
        std::cout << motorvolt.left << ", " << motorvolt.right << std::endl;

        rclcpp::sleep_for(10ms);

        RCLCPP_INFO(this->get_logger(), "Publishing Robot Action");
        auto action = std_msgs::msg::UInt8();
        action.data = 2;
        action_pub_->publish(action);
        std::cout << "Published Robot Action" << std::endl;
    }
};

int main(int argc, const char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TestPub>());
    rclcpp::shutdown();
    return 0;
}