#define SMALL_ENDIAN
// #define BIG_ENDIAN

#include "rclcpp/rclcpp.hpp"
#include "can_msgs/msg/frame.hpp"

#include <stdio.h>
#include <iostream>

#include "canbus_slave/MasterCanProtocol.hpp"
#include "canbus_slave/CanDecode.hpp"

static int can_decode_foc(const std::shared_ptr<can_msgs::msg::Frame> &msg, raw_t *raw)
{
    for (int i = 0; i < CAN_FRAME_DLC; i++)
    {
        raw->raw_data[i] = msg->data[i];
    }
    std::cout << "FOC data in hex: " << std::endl;
    for (int i = 0; i < CAN_FRAME_DLC; i++)
    {
        printf("%02x ", raw->raw_data[i]);
    }
    std::cout << std::endl;

    // decode & store the data
    raw->foc.foc_left = ByteUtil::reconFloat(raw->raw_data, BYTE_POS_FOC_LEFT);
    raw->foc.foc_right = ByteUtil::reconFloat(raw->raw_data, BYTE_POS_FOC_RIGHT);

    std::cout << "FOC decoded data: " << std::endl;
    std::cout << "Left: " << raw->foc.foc_left << std::endl;
    std::cout << "Right: " << raw->foc.foc_right << std::endl;

    return 1;
}

static int can_decode_imu(const std::shared_ptr<can_msgs::msg::Frame> &msg, raw_t *raw)
{
    for (int i = 0; i < CAN_FRAME_DLC; i++)
    {
        raw->raw_data[i] = msg->data[i];
    }
    std::cout << "IMU data in hex: " << std::endl;
    for (int i = 0; i < CAN_FRAME_DLC; i++)
    {
        printf("%02x ", raw->raw_data[i]);
    }
    std::cout << std::endl;

    float temp1 = ByteUtil::reconFloat(raw->raw_data, BYTE_POS_IMU_XZ);
    float temp2 = ByteUtil::reconFloat(raw->raw_data, BYTE_POS_IMU_YW);

    // check type
    switch (msg->id)
    {
    case ID_IMU_ACC_XY:
        raw->imu.acc_x = temp1;
        raw->imu.acc_y = temp2;
        break;
    case ID_IMU_ACC_Z:
        raw->imu.acc_z = temp1;
        break;
    case ID_IMU_ANGVEL_XY:
        raw->imu.angvel_x = temp1;
        raw->imu.angvel_y = temp2;
        break;
    case ID_IMU_ANGVEL_Z:
        raw->imu.angvel_z = temp1;
        break;
    case ID_IMU_ANG_XY:
        raw->imu.ang_x = temp1;
        raw->imu.ang_y = temp2;
        break;
    case ID_IMU_ANG_Z:
        raw->imu.ang_z = temp1;
        break;
    case ID_IMU_QUAT_XY:
        raw->imu.quat_x = temp1;
        raw->imu.quat_y = temp2;
        break;
    case ID_IMU_QUAT_ZW:
        raw->imu.quat_z = temp1;
        raw->imu.quat_w = temp2;
        break;
    default:
        std::cout << "Unknown IMU data type: " << msg->id % 100 << std::endl;
        return -1;
    }

    std::cout << "IMU data type: " << msg->id % 100 << std::endl;
    std::cout << "IMU decoded data: " << std::endl;
    std::cout << "1st data: " << temp1 << " " << "2nd data: " << temp2 << std::endl;

    return 1;
}

int can_decode(const std::shared_ptr<can_msgs::msg::Frame> &msg, raw_t *raw)
{
    auto time = msg->header.stamp;
    std::cout << "Received CAN message at " << time.sec << "s " << time.nanosec << "ns" << std::endl;

    if (msg->dlc != CAN_FRAME_DLC)
    {
        std::cout << "Invalid frame length: " << msg->dlc << std::endl;
        return -1;
    }

    switch (msg->id)
    {
        case ID_IMU_ACC_XY:
        case ID_IMU_ACC_Z:
        case ID_IMU_ANGVEL_XY:
        case ID_IMU_ANGVEL_Z:
        case ID_IMU_ANG_XY:
        case ID_IMU_ANG_Z:
        case ID_IMU_QUAT_XY:
        case ID_IMU_QUAT_ZW:
            std::cout << "IMU data:" << msg->id << std::endl;
            return can_decode_imu(msg, raw);
            break;
        case ID_FOC:
            std::cout << "FOC angle" << std::endl;
            return can_decode_foc(msg, raw);
            break;
        default:
            std::cout << "Unknown message id: " << msg->id << std::endl;
            return -1;
    }
}