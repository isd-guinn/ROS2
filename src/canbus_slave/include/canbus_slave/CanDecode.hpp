#ifndef CAN_DECODE_HPP
#define CAN_DECODE_HPP

#include "canbus_slave/MasterCanProtocol.hpp"
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
    ufloat32_t acc_x; /* acceleration x */
    float acc_y; /* acceleration y */
    float acc_z; /* acceleration z */

    float angvel_x; /* angular velocity x */
    float angvel_y; /* angular velocity y */
    float angvel_z; /* angular velocity z */

    float ang_x; /* angle x */
    float ang_y; /* angle y */
    float ang_z; /* angle z */
};

struct raw_foc_t
{
    float foc_left;  /* left foc angle */
    float foc_right; /* right foc angle */
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
int can_decode(const can_msgs::msg::Frame::SharedPtr msg, raw_t *raw);

#endif