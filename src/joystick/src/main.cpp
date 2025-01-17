#include "rclcpp/rclcpp.hpp"
#include "canbus_slave/msg/foc_angle.hpp"

#include "joystick.h"
#include "joy2volt.h"

#include <unistd.h> // File IO
// #include <fcntl.h>    // File Control & Access Modes
#include <iostream>

using namespace std;

#define AXIS_LR 0 // left right
#define AXIS_UD 1 // up down

const std::string JOYSTICK_DEV = "/dev/input/js0";
int joy_prev[2] = {0, 0}; // left, right
float volt_prev[2] = {0.0f, 0.0f};

class JoystickNode : public rclcpp::Node
{
public:
  Joystick joystick = Joystick(JOYSTICK_DEV);
  JoystickNode()
      : Node("Joystick_node")
  {
    volt_pub_ = this->create_publisher<canbus_slave::msg::FocAngle>("/Motor_volt", 10);
    timer_ = this->create_wall_timer(10ms, std::bind(&JoystickNode::timer_callback, this));
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<canbus_slave::msg::FocAngle>::SharedPtr volt_pub_;

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
        // using Button 3 as reset button (square button)
        if (number == 3 && event.value == 1)
        {
          std::cout << "Button " << static_cast<int>(event.number) << " is " << (event.value == 0 ? "up" : "down") << std::endl;
          // reset button
          joy[0] = 0; joy[1] = 0;
          std::cout << "Reset Joystick" << std::endl;

          // publish the message
          canbus_slave::msg::FocAngle volt_msg;
          volt_msg.left = 0.0f;
          volt_msg.right = 0.0f;
          volt_pub_->publish(volt_msg);
          std::cout << "Pub Voltage Left: " << 0.0f << " Right: " << 0.0f << std::endl;
          
          // update the previous value
          joy_prev[0] = joy[0];
          joy_prev[1] = joy[1];
          for (int i = 0; i < 2; i++)
          {
            // joy_prev[i] = joy[i];
            volt_prev[i] = 0.0f;
          }

          std::cout << "-----------------" << std::endl;
        }
        else return;
      }
      else if (event.isAxis())
      {
        // type = JS_EVENT_AXIS;
        // using the left joystick
        if (number == AXIS_LR || number == AXIS_UD)
        {
          std::cout << "Axis " << static_cast<int>(event.number) << " is at position " << event.value << std::endl;

          // filter
          if ((abs(event.value) < 260) || (abs(event.value) < 2200 && joy_prev[number] == 0))
          {
            joy[number] = 0;
          }
          // else if (abs(event.value) < 2200 && joy_prev[number] == 0)
          // {
          //   joy[number] = 0;
          // }
          else
          {
            joy[number] = event.value;
          }

          float volt[2] = {0.0f, 0.0f};
          joy2theworld(joy[0], joy[1], volt[0], volt[1]);
          for (int i = 0; i < 2; i++)
          {
            if (abs(volt[i]) < 2.0f)
            {
              volt[i] = 0;
            }
          }

          // publish the reading as voltage
          if (volt[number] != volt_prev[number])
          {
            // std::cout << "Voltage Left: " << volt[0] << " Right: " << volt[1] << std::endl;
            canbus_slave::msg::FocAngle volt_msg;
            volt_msg.left = volt[0];
            volt_msg.right = volt[1];
            volt_pub_->publish(volt_msg);
            std::cout << "Pub Voltage Left: " << volt[0] << " Right: " << volt[1] << std::endl;
          }
          // canbus_slave::msg::FocAngle volt_msg;
          // volt_msg.left = volt[0];
          // volt_msg.right = volt[1];
          // std::cout << "Pub Voltage Left: " << volt[0] << " Right: " << volt[1] << std::endl;
          // volt_pub_->publish(volt_msg);

          // update the previous value
          joy_prev[number] = joy[number];
          volt_prev[number] = volt[number];
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

      // // convert to voltage
      // float volt[2] = {0.0f, 0.0f};
      // joy2theworld(joy[0], joy[1], volt[0], volt[1]);

      // // publish the message
      // canbus_slave::msg::FocAngle volt_msg;
      // volt_msg.left = volt[0];
      // volt_msg.right = volt[1];
      // volt_pub_->publish(volt_msg);
      // std::cout << "Pub Voltage Left: " << volt[0] << " Right: " << volt[1] << std::endl;

      // // update the previous value
      // joy_prev[number] = joy[number];
      // return;
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
