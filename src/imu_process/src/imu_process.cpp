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

using namespace std::chrono_literals;
using namespace std;

position_t local_data = {
	{0.0f, 0.0f, 0.0f},	// position in local frame
	{0.0f, 0.0f, 0.0f},	// velocity in local frame
	{0.0f, 0.0f, 0.0f},	// current acceleration
	{0.0f, 0.0f, 0.0f},	// previous acceleration
	0.0f,			// current angle (z-axis)
	0.0f,			// current angular velocity (z-axis)
	0.0f			// previous angular velocity (z-axis)
};	

class IMUProcessor : public rclcpp::Node
{
    public:
		IMUProcessor() : Node("IMU_processor")	
		{	
			rawimu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>("Imu_data", 10, std::bind(&IMUProcessor::rawimu_callback, this, std::placeholders::_1));
			// euler_sub_ = this->create_subscription<serial_imu::msg::EulerAngle>("Imu_euler_angle", 10, std::bind(&IMUProcessor::euler_callback, this, std::placeholders::_1));

			local_pos_pub_ = this->create_publisher<imu_process::msg::Position>("/Imu_local", 20);
			global_pos_pub_ = this->create_publisher<imu_process::msg::Position>("/Imu_global", 20);
			
			timer_ = this->create_wall_timer(500ms, std::bind(&IMUProcessor::timer_callback, this));
		}

    private:
		rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr rawimu_sub_;
		rclcpp::Subscription<serial_imu::msg::EulerAngle>::SharedPtr euler_sub_;
		rclcpp::Publisher<imu_process::msg::Position>::SharedPtr local_pos_pub_;
		rclcpp::Publisher<imu_process::msg::Position>::SharedPtr global_pos_pub_;
        rclcpp::TimerBase::SharedPtr timer_;

		// callback for pub the integrated imu delta-position (x & y)
        void rawimu_callback(const sensor_msgs::msg::Imu::SharedPtr msg){
			// retreive data from msg
			local_data.acc_current.x = precision(msg->linear_acceleration.x, 10);
    		local_data.acc_current.y = precision(msg->linear_acceleration.y, 10);
			local_data.angVel_z_current = precision(msg->angular_velocity.z, 10);
			// process the receive message
			dead_reckon(&local_data);
			update_angle(&local_data);
        }

		/*
		void euler_callback(const serial_imu::msg::EulerAngle::SharedPtr msg){
			// Eigen::Vector3f velocity = Eigen::Vector3f::Zero();
			// Eigen::Vector3f position = Eigen::Vector3f::Zero();

			// for testing eulerToRotationalMatrix()
			// Eigen::Matrix3f rotation_matrix;
			// rotation_matrix.setZero();
			// rotation_matrix = eulerToRotationMatrix(msg->roll_y, msg->pitch_x, msg->yaw_z);
			// std::cout << "Rotation Matrix: " << std::endl << rotation_matrix << std::endl;

			// process the euler angle
			// getGlobalPosition();
		}
		// */

		void timer_callback(){
			auto local = imu_process::msg::Position();
			// auto global = imu_process::msg::Position();

			// publish local position
			local.pos_x = local_data.pos.x;
			local.pos_y = local_data.pos.y;
			local.pos_z = local_data.pos.z;
			local.vel_x = local_data.vel.x;
			local.vel_y = local_data.vel.y;
			local.vel_z = local_data.vel.z;
			local.acc_x = local_data.acc_current.x;
			local.acc_y = local_data.acc_current.y;
			local.acc_z = local_data.acc_current.z;

			local.angle_z = local_data.angle_z;

			local.header.stamp = rclcpp::Clock().now();
			local.header.frame_id = "base_link"; // the frame that this data is associated with
            local_pos_pub_->publish(local); 

			/* publish global position
			global.header.stamp = rclcpp::Clock().now();
			global.header.frame_id = "map"; // the frame that this data is associated with
			// */
			
			// debug
			RCLCPP_INFO(this->get_logger(), "Integrated Local Position -> x: %f, y: %f", local_data.pos.x, local_data.pos.y);
			// RCLCPP_INFO(this->get_logger(), "Integrated Global Position -> x: %f, y: %f", global.pos_x, global.pos_y);
		}
};

int main(int argc,const char* argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<IMUProcessor>());
	rclcpp::shutdown();
	return 0;
}
