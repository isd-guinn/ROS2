CONTROLLER_SERIAL#include "rclcpp/rclcpp.hpp"

#include <unistd.h>   // File IO
#include <fcntl.h>    // File Control & Access Modes
#include <errno.h>
#include <termios.h>  // for terminal operation

#include <stdio.h>   
#include <cstdint> 
#include <chrono>
#include <memory>

#include <iostream>
#include <iomanip>

#define SMALL_ENDIAN
// #define BIG_ENDIAN

#include "uart_slave/ControllerSerialProtocol.hpp"
#include "uart_slave/serial_receive_controller.hpp"
// #include "uart_slave/msg/foc_angle.hpp"

#ifdef __cplusplus
extern "C"{
#endif
#define BAUD                (B115200)
#define CONTROLLER_SERIAL   ("/dev/ttyACM0")
#define DEG_TO_RAD          (0.01745329)
#ifdef __cplusplus
}
#endif

using namespace std::chrono_literals;
using namespace std;
static raw_t raw; // struct for storing the raw data from the serial port

class UartControllerReceiver : public rclcpp::Node
{
public:
    int uart_fd_ = 0;
    uint8_t Rx_buffer[C2M_POCKET_SIZE] = {0};
    UartControllerReceiver()
        : Node("Uart_receiver_controller")
    {
        uart_fd_ = open_serial();
        uart_pub_motorvoltage_ = this->create_publisher<uart_slave::msg::FocAngle>("/Motor_voltage", 10);
        timer_ = this->create_wall_timer(1000ms, std::bind(&UartControllerReceiver::timer_callback, this));
    }
    ~UartControllerReceiver()
    {
        if (uart_fd_ != -1)
        {
            close(uart_fd_);
        }
    }

private:
    rclcpp::Publisher<uart_slave::msg::FocAngle>::SharedPtr uart_pub_motorvoltage_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    int num_bytes;

    void timer_callback(){
        auto motor_voltage = uart_slave::msg::FocAngle();
        num_bytes = read(uart_fd_, Rx_buffer, sizeof(Rx_buffer));
        // now the data is in Rx_buffer
        // num_bytes is the number of bytes received

        // store the raw data into the raw struct
        int rev = serial_input(&raw, Rx_buffer, num_bytes);

        if (rev){
            // successfully decoded the data
            motor_voltage.left = raw.MotorVolt_L;
            motor_voltage.right = raw.MotorVolt_R;
            uart_pub_motorvoltage_->publish(motor_voltage);

            std::cout << "Bytes of data received: ";
            for (int i=0; i < C2M_PACKET_SIZE; i++)
            {
                std::cout << std::hex << static_cast<int>(Rx_buffer[i]) << " ";
            }
            std::cout << std::endl;
        }
        else {
            std::cout << "No data is received." << std::endl;
        }

        // for preparing to receive the next data
		memset(Rx_buffer,0,sizeof(Rx_buffer));
    }

    int open_serial(void)
		{
			struct termios options;

			int fd = open(CONTROLLER_SERIAL, O_RDWR | O_NOCTTY);
			if(fd == -1)
			{
				perror("unable to open serial port");
				exit(0);
			}
			
            tcgetattr(uart_fd_, &options);
			memset(&options, 0, sizeof(options));

			options.c_cflag = BAUD | CS8 | CLOCAL | CREAD;
			options.c_iflag = IGNPAR;
			options.c_oflag = 0;
			options.c_lflag = 0;
			options.c_cc[VTIME] = 0;
			options.c_cc[VMIN] = 0;
			tcflush(fd, TCIFLUSH);
			tcsetattr(fd, TCSANOW, &options);

			return fd;
		}
};

int main(int argc, const char * argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<UartControllerReceiver>());
	rclcpp::shutdown();
	return 0;
}