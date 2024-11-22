#include "rclcpp/rclcpp.hpp"
#include "uart_slave/msg/foc_angle.hpp"

#include "joystick.h"
#include "joy2volt.h"

#include <unistd.h> // File IO
// #include <fcntl.h>    // File Control & Access Modes
#include <iostream>

using namespace std;

const std::string JOYSTICK_DEV = "/dev/input/js0";
int joy_prev[2] = {0, 0}; // left, right

class JoystickNode : public rclcpp::Node
{
public:
  Joystick joystick = Joystick(JOYSTICK_DEV);
  JoystickNode()
      : Node("Joystick_node")
  {
    volt_pub_ = this->create_publisher<uart_slave::msg::FocAngle>("/Motor_voltage", 10);
    timer_ = this->create_wall_timer(10ms, std::bind(&JoystickNode::timer_callback, this));
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<uart_slave::msg::FocAngle>::SharedPtr volt_pub_;

  void timer_callback()
  {
    if (!joystick.isFound())
    {
      std::cout << "Joystick not found." << std::endl;
      exit(1);
    }

    // Attempt to sample an event from the joystick
    JoystickEvent event;
    if (joystick.sample(&event))
    {
      // int type = 0;
      int number = static_cast<int>(event.number);
      int joy[2] = {joy_prev[0], joy_prev[1]};
      if (event.isButton())
      {
        // type = JS_EVENT_BUTTON;
        std::cout << "Button " << static_cast<int>(event.number) << " is " << (event.value == 0 ? "up" : "down") << std::endl;
        if (number == 13 && event.value == 1)
        {
          // reset button
          joy[0] = 0; joy[1] = 0;
          std::cout << "Reset Joystick" << std::endl;
        }
        else return;
      }
      else if (event.isAxis())
      {
        // type = JS_EVENT_AXIS;
        std::cout << "Axis " << static_cast<int>(event.number) << " is at position " << event.value << std::endl;
        if (number == 0 || number == 1)
        {
          // update value
          // std::cout << "prev: " << joy_prev[0] << " " << joy_prev[1] << std::endl;
          joy[number] = event.value;
          // std::cout << "current: " << joy[0] << " " << joy[1] << std::endl;

          float volt[2] = {0.0f, 0.0f};
          joy2theworld(joy[0], joy[1], volt[0], volt[1]);

          // publish the reading as voltage
          uart_slave::msg::FocAngle volt_msg;
          volt_msg.left = volt[0];
          volt_msg.right = volt[1];
          std::cout << "Pub Voltage Left: " << volt[0] << " Right: " << volt[1] << std::endl;
          volt_pub_->publish(volt_msg);

          // update the previous value
          joy_prev[number] = joy[number];
        }
        else return;
      }
      else if (event.isInitialState())
      {
        std::cout << "Initial state of button " << static_cast<int>(event.number) << " is " << (event.value == 0 ? "up" : "down") << std::endl;
        return;
      }
      else
      {
        std::cout << "Unknown event type: " << event.type << std::endl;
        return;
      }

      // convert to voltage
      float volt[2] = {0.0f, 0.0f};
      joy2theworld(joy[0], joy[1], volt[0], volt[1]);

      // publish the message
      uart_slave::msg::FocAngle volt_msg;
      volt_msg.left = volt[0];
      volt_msg.right = volt[1];
      volt_pub_->publish(volt_msg);
      std::cout << "Pub Voltage Left: " << volt[0] << " Right: " << volt[1] << std::endl;

      // update the previous value
      joy_prev[number] = joy[number];
      return;
    }
  }
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JoystickNode>());
  rclcpp::shutdown();
  return 0;
}
