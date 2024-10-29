#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <sensor_msgs/msg/imu.hpp>
#include "serial_imu/msg/euler_angle.hpp"

#include "builtin_interfaces/msg/time.hpp"
#include "std_msgs/msg/header.hpp"
#include "imu_process/msg/position.hpp"

#include <iostream>
#include <memory>
#include <unistd.h>
#include <fstream>

#include "imu_process/manipulate.hpp"
#include <eigen3/Eigen/Dense>

using namespace std::chrono_literals;
using namespace std;

position_t local_data;

// double last_update_time = 0;

class IMUProcessor : public rclcpp::Node
{
    public:
		IMUProcessor() : Node("IMU_processor")	
		{	
			rawimu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>("Imu_data", 10, std::bind(&IMUProcessor::rawimu_callback, this, std::placeholders::_1));
			local_pos_pub_ = this->create_publisher<imu_process::msg::Position>("/Imu_local", 20);
			global_pos_pub_ = this->create_publisher<imu_process::msg::Position>("/Imu_global", 20);
			
			timer_ = this->create_wall_timer(500ms, std::bind(&IMUProcessor::timer_callback, this));
		}
		double last_update_time = 0;

    private:
		rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr rawimu_sub_;
		rclcpp::Subscription<serial_imu::msg::EulerAngle>::SharedPtr euler_sub_;
		rclcpp::Publisher<imu_process::msg::Position>::SharedPtr local_pos_pub_;
		rclcpp::Publisher<imu_process::msg::Position>::SharedPtr global_pos_pub_;
        rclcpp::TimerBase::SharedPtr timer_;

		// callback for pub the integrated imu delta-position (x & y)
        void rawimu_callback(const sensor_msgs::msg::Imu::SharedPtr msg){
			// retreive data from msg
			local_data.acc_measured << 	precision(msg->linear_acceleration.x, 10), 
										precision(msg->linear_acceleration.y, 10), 
										precision(msg->linear_acceleration.z, 10);
			local_data.quat.w() = precision(msg->orientation.w, 10);
			local_data.quat.x() = precision(msg->orientation.x, 10);
			local_data.quat.y() = precision(msg->orientation.y, 10);
			local_data.quat.z() = precision(msg->orientation.z, 10);
			// local_data.quat = (precision(msg->orientation.w, 10),
			// 					precision(msg->orientation.x, 10), 
			// 					precision(msg->orientation.y, 10), 
			// 					precision(msg->orientation.z, 10));
			// local_data.acc_measured.x = precision(msg->linear_acceleration.x, 10);
    		// local_data.acc_measured.y = precision(msg->linear_acceleration.y, 10);
			// local_data.quat.x = precision(msg->orientation.x, 10);
			// local_data.quat.y = precision(msg->orientation.y, 10);
			// local_data.quat.z = precision(msg->orientation.z, 10);
			// local_data.quat_w = precision(msg->orientation.w, 10);
			local_data.angVel_z_current = precision(msg->angular_velocity.z, 10);
			std::cout << "---------------------------" << std::endl;

			std::cout << "Measured Acceleration: " << local_data.acc_measured.transpose() << std::endl;
			Eigen::Matrix3f RotationalMatrix = quaternionToRotationMatrix(local_data.quat);
			std::cout << "Quaternion: " << local_data.quat << std::endl;
			std::cout << "Rotational Matrix: " << std::endl;
			std::cout << RotationalMatrix << std::endl;

			// Eigen::Array3f gravity_local = distributeGravity(RotationalMatrix);
			// std::cout << "Gravity in local frame: " << gravity_local << std::endl;

			// process the receive message
			double elapse_time = dead_reckon(&local_data, last_update_time);
			// std::cout << "callback last update time: " << last_update_time << std::endl;
			update_angle(&local_data, elapse_time);

			std::cout << "Final Position: " << local_data.pos_final.transpose() << std::endl;
			std::cout << "Final Velocity: " << local_data.vel_final.transpose() << std::endl;
			std::cout << "Final Acceleration: " << local_data.acc_final.transpose() << std::endl;
			std::cout << "---------------------------" << std::endl;

        }

		void timer_callback(){
			auto local = imu_process::msg::Position();
			// auto global = imu_process::msg::Position();

			// publish local position
			local.pos_x = local_data.pos_final(0);
			local.pos_y = local_data.pos_final(1);
			local.pos_z = local_data.pos_final(2);
			local.vel_x = local_data.vel_final(0);
			local.vel_y = local_data.vel_final(1);
			local.vel_z = local_data.vel_final(2);
			local.acc_x = local_data.acc_final(0);
			local.acc_y = local_data.acc_final(1);
			local.acc_z = local_data.acc_final(2);

			local.angle_z = local_data.angle_z;

			local.header.stamp = rclcpp::Clock().now();
			local.header.frame_id = "base_link"; // the frame that this data is associated with
            local_pos_pub_->publish(local); 
		}
};

int main(int argc,const char* argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<IMUProcessor>());
	rclcpp::shutdown();
	return 0;
}
