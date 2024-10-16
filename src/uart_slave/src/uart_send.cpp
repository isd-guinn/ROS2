#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include <unistd.h>   // File IO
#include <fcntl.h>    // File Control & Access Modes
#include <errno.h>
#include <termios.h>  // for terminal operation

#include <chrono>
#include <memory>

#include "uart_slave/SerialCommunication.hpp"
// #include <wiringPi.h>
// #include <wiringSerial.h>

#ifdef __cplusplus
extern "C"{
#endif

#define BAUD          (B115200)
#define SLAVE_SERIAL  ("/dev/ttyAMA0") 
// if on-board UART: "/dev/ttyAMA10" equals to "/dev/serial0" - debug UART port
/* 
to use GPIO pin instead ("/dev/ttyAMA0"), 
sudo nano /boot/config.txt
sudo nano /boot/cmdline.txt
go to raspi-config and enable serial port -> reboot 
now config.txt should have dtparam=uart0=on
and ttyAMA0 should show up in /dev
*/
#ifdef __cplusplus
}
#endif

using namespace std::chrono_literals;
using namespace std;

class UartPublisher : public rclcpp::Node
{
public:
    int uart_fd_ = 0;
    UartPublisher()
        : Node("Uart_sender")
    {
        uart_fd_ = open_serial();
        // uart_sub_imuraw_ = this->create_publisher<sensor_msgs::msg::Imu>("Imu_data", 10, imu_callback);
        // uart_sub_algo_ = this->create_publisher<???>("???", 10, algo_callback);
        timer_ = this->create_wall_timer(
            1000ms, std::bind(&UartPublisher::timer_callback, this));
    }

    ~UartPublisher() // destructor
    {
        if (uart_fd_ != -1)
        {
            close(uart_fd_);
        }
    }

private:
  void timer_callback()
  {
    uint8_t data[POCKET_SIZE];

    // prepare the pocket
    data[0] = '>'; // StartBit
    data[1] = EstopDisabledBit; // EStopBit
    data[2] = WBDirStopBit; // WBDirBit
    data[POCKET_SIZE-1] = 0x00; // reset the value
    // Calculate checksum
    for (int i = 0; i < POCKET_SIZE - 1; i++) {
        data[POCKET_SIZE - 1] += data[i]; // check sum
    }

    // Write to Serial Port
    ssize_t bytes_written = write(uart_fd_, data, sizeof(data));
    if (bytes_written == -1) {
        RCLCPP_ERROR(this->get_logger(), "Failed to write to serial port: %s", strerror(errno));
    }
    else
    {
      RCLCPP_INFO(this->get_logger(), "Wrote %ld bytes to serial port", bytes_written);
      // for debug: display the data
      for (int i = 0; i < POCKET_SIZE; i++)
      {
        RCLCPP_INFO(this->get_logger(), "wrote %x ", data[i]);
      }
    }
  }

  // void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg){
  //   // get the data from the topic
  // }

  // void algo_callback(const ??? msg){
  //   // get the data from the topic
  // }

  int open_serial(void)
  {
    struct termios options;

    // Open Serial Port
    int uart_fd_ = open(SLAVE_SERIAL, O_RDWR | O_NOCTTY);
    if (uart_fd_ == -1)
    {
      perror("unable to open serial port");
      exit(0);
    }

    tcgetattr(uart_fd_, &options);
    memset(&options, 0, sizeof(options));

    // Configure UART
    // baud rate | character size = 8 bits | ignore modem control lines | enable receiver
    options.c_cflag = BAUD | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR; // ignore framing & parity errors
    options.c_oflag = 0; // no special output processing
    options.c_lflag = 0; // no special local processing (including echo)
    tcflush(uart_fd_, TCIFLUSH); // flush data that're received but not read
    tcsetattr(uart_fd_, TCSANOW, &options); // changes occur immediately

    return uart_fd_;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  // rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr uart_sub_imuraw_;
  // rclcpp::Subscription<???>::SharedPtr uart_sub_algo_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<UartPublisher>());
  rclcpp::shutdown();
  return 0;
}