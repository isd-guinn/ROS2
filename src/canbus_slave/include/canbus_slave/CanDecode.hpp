#ifndef CAN_DECODE_HPP
#define CAN_DECODE_HPP

#include "canbus_slave/MasterCanProtocol.hpp"
#include <can_msgs/msg/frame.hpp>

#include <stdint.h>

namespace ByteUtil
{
  // reinterpret into float
  inline float reconFloat(uint8_t *packet, uint8_t pos, bool isSmallEndian = true)
  {
    if (isSmallEndian)
    {
      uint8_t ctn[4] = {packet[pos+3], packet[pos+2], packet[pos+1], packet[pos]};
      return *(float*)&ctn;
    }
    else
    {
      uint8_t ctn[4] = {packet[pos], packet[pos+1], packet[pos+2], packet[pos+3]};
      return *(float*)&ctn;
    }              
  }
}

struct raw_imu_t
{
    float acc_x = 0.0; /* acceleration x */
    float acc_y = 0.0; /* acceleration y */
    float acc_z = 0.0; /* acceleration z */

    float angvel_x = 0.0; /* angular velocity x */
    float angvel_y = 0.0; /* angular velocity y */
    float angvel_z = 0.0; /* angular velocity z */

    float ang_x = 0.0; /* angle x */
    float ang_y = 0.0; /* angle y */
    float ang_z = 0.0; /* angle z */

    float quat_x = 0.0; /* quaternion x */
    float quat_y = 0.0; /* quaternion y */
    float quat_z = 0.0; /* quaternion z */
    float quat_w = 0.0; /* quaternion w */
};

struct raw_foc_t
{
    float foc_left = 0.0;  /* left foc angle */
    float foc_right = 0.0; /* right foc angle */
};

struct raw_t
{
    uint8_t raw_data[CAN_FRAME_DLC]; /* raw data */
    raw_foc_t foc;
    raw_imu_t imu;
};

/* Update raw_t struct with the received data
 * @return   status (-1: error message, 1: input data successfully)
 */
int can_decode(const std::shared_ptr<can_msgs::msg::Frame>& msg, raw_t *raw);

#endif