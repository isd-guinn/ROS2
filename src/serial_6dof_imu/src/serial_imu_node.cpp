// done porting
/******************************************************************
 * Copyright:2016-2021 www.corvin.cn ROS小课堂
 * Description:使用串口来读取IMU的数据,并通过ROS话题topic将数据发布出来.
 *   芯片中默认读取出来的加速度数据单位是g,需要将其转换为ROS中加速度规定的
 *   m/s2才能发布.ROS中使用的坐标系为ENU东北天坐标系,x轴指向东,y轴指向北,
 *   z轴指向天空.
 * Author: corvin
 * History:
 *   20211122:init this file.
 *   20220220:ROS中IMU的坐标系为ENU东北天坐标系,所以需要修改线加速度的
 *       数据正方向,与XYZ三轴正方向都相反,此时Z轴加速度G为正值.
******************************************************************/
#include <memory>

// #include <ros/ros.h>
#include "rclcpp/rclcpp.hpp"
// not yet update
// #include <tf/tf.h> // 机器人坐标系管理
#include <tf2/tf2.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
// #include <sensor_msgs/Imu.h>
#include "sensor_msgs/msg/Imu.hpp"
// #include <std_msgs/Float32.h>
// #include <std_msgs/Empty.h>
#include "std_msgs/msgs/Float32.hpp"
#include "std_msgs/msgs/Empty.hpp"
// not yet update
#include <imu_data.hpp> // where is this source file?
#include "serial_6dof_imu/srv/setYawZero.hpp"
#include "serial_6dof_imu/srv/getYawData.hpp"
#include "serial_6dof_imu/srv/setIICAddr.hpp"

static float g_yawData;  // Global variable storing current yaw value

/**********************************************************
 * Description:将yaw角度归零的话题回调函数,只要往话题中发布一条
 *   std_msgs::Empty类型消息,即可将yaw角度归零.
 *********************************************************/
// Callback function for yaw zeroing
void yawZeroCallback(const std_msgs::msg::Empty::SharedPtr msg)
{
    makeYawZero();
}

/******************************************************************
 * Description:使用service方式来进行yaw角度归零,这里是服务的回调
 *   函数,当有客户端发送yaw归零的服务时,自动调用该函数,如果正确
 *   执行了yaw归零,则response反馈为0,如果为其他负数则表明yaw归零
 *   命令执行失败.
 *****************************************************************/
// Service to zero yaw
// bool yawZeroService(serial_6dof_imu::setYawZero::Request &req,
//                     serial_6dof_imu::setYawZero::Response &res)
// {
//     res.status = makeYawZero();
//     return true;
// }
bool yawZeroService(const std::shared_ptr<serial_6dof_imu::srv::setYawZero::Request> req,
                    std::shared_ptr<serial_6dof_imu::srv::setYawZero::Response> res)
{
    res->status = makeYawZero();
    return true;
}

/**********************************************************************
    Service to get yaw data
 **********************************************************************/
// bool getYawDataService(serial_6dof_imu::getYawData::Request &req,
//                        serial_6dof_imu::getYawData::Response &res)
// {
//     res.yaw = g_yawData;
//     return true;
// }

void getYawDataService(const std::shared_ptr<serial_6dof_imu::srv::getYawData::Request> req,
                       std::shared_ptr<serial_6dof_imu::srv::getYawData::Response> res)
{
    res->yaw = g_yawData;
}

/*******************************************************************
 * Description:通过service方式来更新IIC地址,默认地址为0x50,可以更新的地址
 *   范围为0x00 - 0x7F,注意更新的地址不能与IIC总线上其他设备地址冲突,这样才
 *   能在IIC总线上读取到所有地址上IIC设备的数据.
 ******************************************************************/
// bool setIICAddrService(serial_6dof_imu::setIICAddr::Request &req,
//                        serial_6dof_imu::setIICAddr::Response &res)
// {
//     res.status = updateIICAddr(req.address);
//     return true;
// }

bool setIICAddrService(const std::shared_ptr<serial_6dof_imu::srv::setIICAddr::Request> req,
                       std::shared_ptr<serial_6dof_imu::srv::setIICAddr::Response> res)
{
    res->status = updateIICAddr(req->address);
    return true;
}

/********************************************************************
 * Description:入口主函数,负责通过topic和service来发布处理数据.
 *******************************************************************/

int main(int argc, char **argv)
{
    // node name is imu_data_pub_node
    // ros::init(argc, argv, "imu_data_pub_node");
    // ros::NodeHandle handle;
    rclcpp::init(argc, argv);
    auto handle = rclcpp::Node::make_shared("imu_data_pub_node");

    float yaw, pitch, roll;
    std::string imu_dev;
    std::string imu_link_name;
    std::string imu_topic_name;
    std::string yaw_pub_topic;
    std::string pitch_pub_topic;
    std::string roll_pub_topic;
    std::string temp_pub_topic;
    std::string yaw_zero_topic;
    std::string yaw_zero_service;
    std::string yaw_data_service;
    std::string set_iic_addr_service;

    int pub_topic_hz = 0;  //话题发布imu数据的频率
    const float degree2Rad = 0.017453292;  //3.1415926/180.0,将角度转换为弧度的系数
    const float acc_factor = 9.806; //重力加速度常量

    //launch文件中加载yaml配置文件，然后从yaml配置文件中读取各参数
    // not yet update
    ros::param::get("~imu_dev",          imu_dev);
    ros::param::get("~imu_link_name",    imu_link_name);
    ros::param::get("~pub_topic_hz",     pub_topic_hz);
    ros::param::get("~pub_data_topic",   imu_topic_name);
    ros::param::get("~yaw_zero_topic",   yaw_zero_topic);
    ros::param::get("~yaw_zero_service", yaw_zero_service);
    ros::param::get("~yaw_pub_topic",    yaw_pub_topic);
    ros::param::get("~temp_pub_topic",   temp_pub_topic);
    ros::param::get("~pitch_pub_topic",  pitch_pub_topic);
    ros::param::get("~roll_pub_topic",   roll_pub_topic);
    ros::param::get("~get_yaw_data_srv", yaw_data_service);
    ros::param::get("~set_iic_addr_srv", set_iic_addr_service);

    // Declare and get parameters
    handle->declare_parameter<std::string>("imu_dev", "/dev/ttyUSB0");
    handle->declare_parameter<std::string>("imu_link_name", "imu_link");
    handle->declare_parameter<int>("pub_topic_hz", 50);
    handle->declare_parameter<std::string>("pub_data_topic", "imu/data");
    handle->declare_parameter<std::string>("yaw_zero_topic", "imu/yaw_zero");
    handle->declare_parameter<std::string>("yaw_zero_service", "set_yaw_zero");
    handle->declare_parameter<std::string>("yaw_pub_topic", "imu/yaw");
    handle->declare_parameter<std::string>("pitch_pub_topic", "imu/pitch");
    handle->declare_parameter<std::string>("roll_pub_topic", "imu/roll");
    handle->declare_parameter<std::string>("temp_pub_topic", "imu/temp");
    handle->declare_parameter<std::string>("get_yaw_data_srv", "get_yaw_data");
    handle->declare_parameter<std::string>("set_iic_addr_srv", "set_iic_addr");

    //初始化imu模块串口,根据设备号与IMU建立连接
    int ret = initSerialPort(imu_dev.c_str());
    if(ret < 0) //通过串口连接IMU模块失败
    {
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "init 6DOF IMU module serial port error !");
        closeSerialPort();
        return -1;
    }
    RCLCPP_INFO(rclcpp::get_logger("imu_data_pub_node"), "Now 6DOF IMU module is working...");

    //定义服务,分别是yaw角度归零和获取yaw当前角度,更新IIC地址
    // ros::ServiceServer setyawSrv  = handle.advertiseService(yaw_zero_service,  yawZeroService);
    // ros::ServiceServer getYawSrv  = handle.advertiseService(yaw_data_service,  getYawDataService);
    // ros::ServiceServer setIICSrv  = handle.advertiseService(set_iic_addr_service, setIICAddrService);
    auto setYawSrv = handle->create_service<serial_6dof_imu::srv::setYawZero>("yaw_zero_service", yawZeroService);
    auto getYawSrv = handle->create_service<serial_6dof_imu::srv::getYawData>("yaw_data_service", getYawDataService);
    auto setIICSrv = handle->create_service<serial_6dof_imu::srv::setIICAddr>("set_iic_addr_service", setIICAddrService);

    // ros::Subscriber yawZeroSub = handle.subscribe(yaw_zero_topic, 1, yawZeroCallback);
    auto yawZeroSub = handle->create_subscription<std_msgs::msg::Float32>("yaw_zero_topic", 1, yawZeroCallback);
    // ros::Publisher imu_pub   = handle.advertise<sensor_msgs::Imu>(imu_topic_name, 2);
    // ros::Publisher yaw_pub   = handle.advertise<std_msgs::Float32>(yaw_pub_topic, 2);
    // ros::Publisher pitch_pub = handle.advertise<std_msgs::Float32>(pitch_pub_topic, 2);
    // ros::Publisher temp_pub  = handle.advertise<std_msgs::Float32>(temp_pub_topic, 2);
    // ros::Publisher roll_pub  = handle.advertise<std_msgs::Float32>(roll_pub_topic, 2);
    auto imu_pub = handle->create_publisher<sensor_msgs::msg::Imu>("imu_topic_name", 2);
    auto yaw_pub = handle->create_publisher<std_msgs::msg::Float32>("yaw_pub_topic", 2);
    auto pitch_pub = handle->create_publisher<std_msgs::msg::Float32>("pitch_pub_topic", 2);
    auto roll_pub = handle->create_publisher<std_msgs::msg::Float32>("roll_pub_topic", 2);
    auto temp_pub = handle->create_publisher<std_msgs::msg::Float32>("temp_pub_topic", 2);
    // ros::Rate loop_rate(pub_topic_hz);
    rclcpp::Rate loop_rate(pub_topic_hz);

    sensor_msgs::msg::Imu imu_msg;
    imu_msg.header.frame_id = imu_link_name;
    std_msgs::msg::Float32 yaw_msg;
    std_msgs::msg::Float32 pitch_msg;
    std_msgs::msg::Float32 roll_msg;
    std_msgs::msg::Float32 temp_msg;

    // the publishing loop
    while(rclcpp::ok())
    {
        if(getImuData() < 0)
            break;

        // imu_msg.header.stamp = ros::Time::now();
        imu_msg.header.stamp = rclcpp::Time::now();
        roll  = getAngle(0)*degree2Rad;
        pitch = getAngle(1)*degree2Rad;
        yaw   = getAngle(2)*degree2Rad;
        if(yaw >= 3.1415926)
            yaw -= 6.2831852;

        g_yawData = yaw; //获取yaw值,可以通过服务来得到该值
        yaw_msg.data   = yaw;
        pitch_msg.data = pitch;
        roll_msg.data  = roll;
        temp_msg.data  = getTemp();
        yaw_pub->publish(yaw_msg);     //将yaw值通过话题发布出去
        pitch_pub->publish(pitch_msg); //将pitch值通过话题发布出去
        roll_pub->publish(roll_msg);   //将roll值通过话题发布出去
        temp_pub->publish(temp_msg);   //将temp值通过话题发布出去

        //ROS_INFO("yaw:%f pitch:%f roll:%f",yaw, pitch, roll);
        RCLCPP_INFO(rclcpp::get_logger("imu_data_pub_node"), "yaw:%f pitch:%f roll:%f", yaw, pitch, roll);
        imu_msg.orientation.x = getQuat(1);
        imu_msg.orientation.y = getQuat(2);
        imu_msg.orientation.z = getQuat(3);
        imu_msg.orientation.w = getQuat(0);
        imu_msg.orientation_covariance = {0, 0, 0,
                                          0, 0, 0,
                                          0, 0, 0};
        //三轴角速度
        imu_msg.angular_velocity.x = getAngular(0)*degree2Rad;
        imu_msg.angular_velocity.y = getAngular(1)*degree2Rad;
        imu_msg.angular_velocity.z = getAngular(2)*degree2Rad;
        imu_msg.angular_velocity_covariance = {0, 0, 0,
                                               0, 0, 0,
                                               0, 0, 0};
        //三轴线加速度,这里规定xyz三轴线加速度坐标系为ENU
        imu_msg.linear_acceleration.x = getAcc(0)*acc_factor;
        imu_msg.linear_acceleration.y = getAcc(1)*acc_factor;
        imu_msg.linear_acceleration.z = getAcc(2)*acc_factor;
        imu_msg.linear_acceleration_covariance = {0, 0, 0,
                                                  0, 0, 0,
                                                  0, 0, 0};

        imu_pub->publish(imu_msg); //将imu数据通过话题发布出去
        // ros::spinOnce();
        rclcpp::spin_some(handle);
        loop_rate.sleep();
    }

    closeSerialPort(); //关闭与IMU模块的串口连接
    return 0;
}
