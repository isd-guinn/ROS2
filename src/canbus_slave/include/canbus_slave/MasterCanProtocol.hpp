#ifndef MASTER_CAN_PROTOCOL_HPP  
#define MASTER_CAN_PROTOCOL_HPP

// #ifdef BIG_ENDIAN
// #define EXTRACT_BYTE_FROM_4BYTE_VALUE(x,y) (*((uint8_t*)(&x)+y))
// #endif

#ifdef SMALL_ENDIAN
#define EXTRACT_BYTE_FROM_4BYTE_VALUE(x,y) (*((uint8_t*)(&x)+(3-(y))))
#endif

/* MAX 8 BYTES USING CAN2.0 */
#define CAN_FRAME_DLC 8

/* FRAME ID - S2M */
#define ID_IMU_DATA_ACC_XY      101
#define ID_IMU_DATA_ACC_Z       102
#define ID_IMU_DATA_ANGVEL_XY   103
#define ID_IMU_DATA_ANGVEL_Z    104
#define ID_IMU_DATA_ANG_XY      105
#define ID_IMU_DATA_ANG_Z       106
#define ID_IMU_DATA_QUAT_XY     107
#define ID_IMU_DATA_QUAT_ZW     108

#define ID_FOC_ANGLE            200

/*  Byte Position Macros - IMU DATA

  00  |   Data_x OR Data_z  (1st Byte)
  01  |   Data_x OR Data_z  (2nd Byte)
  02  |   Data_x OR Data_z  (3rd Byte)
  03  |   Data_x OR Data_z  (4th Byte)

  04  |   Data_y OR Data_w  (1st Byte)
  05  |   Data_y OR Data_w  (2nd Byte)
  06  |   Data_y OR Data_w  (3rd Byte)
  07  |   Data_y OR Data_w  (4th Byte)
*/

/*  Byte Position Macros - FOC ANGLE
  
  00  |   FOC Angle Left    (1st Byte)
  01  |   FOC Angle Left    (2nd Byte)
  02  |   FOC Angle Left    (3rd Byte)
  03  |   FOC Angle Left    (4th Byte)

  04  |   FOC Angle Right   (1st Byte)
  05  |   FOC Angle Right   (2nd Byte)
  06  |   FOC Angle Right   (3rd Byte)
  07  |   FOC Angle Right   (4th Byte)
*/

/* IMU DATA BYTE */
#define IMU_DATA_X_Z 0
#define IMU_DATA_Y_W 4

/* FOC ANGLE */
#define FOC_ANGLE_LEFT 0
#define FOC_ANGLE_RIGHT 4

#endif