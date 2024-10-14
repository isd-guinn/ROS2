#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <sensor_msgs/msg/imu.hpp>

#include <memory>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <fstream>

// create a dummy share pointer node from rclcpp::Node for subscription of raw data
// rclcpp::Node::SharedPtr nh = nullptr;
using namespace std::chrono_literals;
using namespace std;
using json = nlohmann::json;

const std::string position_file = "/position.json";
static float pos_x, pos_y;
static float vel_x, vel_y;
static float acc_x, acc_y;

float precision( float f, int places )
{
    float n = std::pow(10.0f, places ) ;
    return std::round(f * n) / n ;
}

// create node class "IMUProcessor"
class IMUProcessor : public rclcpp::Node
{
    public:
		// name the node as "IMU_processor"
		IMUProcessor() : Node("IMU_processor")	
		{	
			// pub message type NOT IMU! -> self-defined type, pub topic name = "Imu_processed"
			imu_processed_pub_ = this->create_publisher<std_msgs::msg::String>("Imu_processed", 20);
			// sub message type = Imu, sub topic name = "Imu_data"
			imu_processed_sub_ = this->create_subscription<sensor_msgs::msg::Imu>("Imu_data", 10, std::bind(&IMUProcessor::topic_callback, this, std::placeholders::_1));
			// timer_callback function to be init every 2ms -> 500ms for testing purpose
			timer_ = this->create_wall_timer(500ms, std::bind(&IMUProcessor::timer_callback, this));
		}
    private:
		// integration function for dead reckoning
		void manipulate(const sensor_msgs::msg::Imu::SharedPtr msg){
			// Placeholder for dead reckoning integration logic
			// Initialize variables for position, velocity, and acceleration

			double dt = 0.1; // 100ms time step (assuming constant for simplicity)

			// Read the position data from the file
			std::ifstream infile(position_file);
			if (infile.is_open()) {
				json j;
				infile >> j;
				pos_x = j["pos_x"];				pos_y = j["pos_y"];
				vel_x = j["vel_x"];				vel_y = j["vel_y"];
				infile.close();
			} else {
				std::cerr << "Unable to open file for reading: " << position_file << std::endl;
			}

			// Update acceleration from IMU data
			acc_x = precision(msg->linear_acceleration.x, 18);
			acc_y = precision(msg->linear_acceleration.y, 18);

			// Integrate acceleration to get velocity
			vel_x += acc_x * dt;
			vel_y += acc_y * dt;

			// Integrate velocity to get position
			pos_x += vel_x * dt;
			pos_y += vel_y * dt;

			// Print the current position for debugging
			RCLCPP_INFO(this->get_logger(), "Integrated Position -> x: %f, y: %f", pos_x, pos_y);

			// filter out noise & drift
			
			// Write the updated position data to the file
			std::ofstream outfile(position_file);
			if (outfile.is_open()) {
				json j;
				j["pos_x"] = pos_x;				j["pos_y"] = pos_y;
				j["vel_x"] = vel_x;				j["vel_y"] = vel_y;
				outfile << j.dump();
				outfile.close();
			} else {
				std::cerr << "Unable to open file for writing: " << position_file << std::endl;
			}
		}

		// callback for pub the integrated imu delta-position (x & y)
        void topic_callback(const sensor_msgs::msg::Imu::SharedPtr msg){
			// process the receive message
			manipulate(msg);

            // message is named "imu_processed" with message type of "imu"
            auto imu_processed = std_msgs::msg::String();
			// set the message data
			imu_processed.data = "pos_x testing";
			// publish the processed message
            imu_processed_pub_->publish(imu_processed);
        }

		void timer_callback(){
			// periodic callback function to read the current position
			std::ifstream infile(position_file);
			if (infile.is_open()) {
				json j;
				infile >> j;
				float pos_x = j["pos_x"];
				float pos_y = j["pos_y"];
				infile.close();

				// Log the current position
				RCLCPP_INFO(this->get_logger(), "Timer Callback - Current Position -> x: %f, y: %f", pos_x, pos_y);
			} else {
				std::cerr << "Unable to open file for reading: " << position_file << std::endl;
			}
		}

		rclcpp::Publisher<std_msgs::msg::String>::SharedPtr imu_processed_pub_;
		rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_processed_sub_;
        rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc,const char* argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<IMUProcessor>());
	rclcpp::shutdown();
	return 0;
}
