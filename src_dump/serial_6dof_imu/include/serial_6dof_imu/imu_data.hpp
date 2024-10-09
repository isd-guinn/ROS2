/*******************************************************
 * Copyright:2016-2021 www.corvin.cn ROS小课堂
 * Description:使用串口操作读取imu模块的代码头文件.
 * Author: corvin
 * History:
 *   20211122:init this file.
 *******************************************************/
#ifndef _IMU_DATA_H_
#define _IMU_DATA_H_

int initSerialPort(const char* path);

int getImuData(void);
int closeSerialPort(void);

float getAcc(int flag);
float getAngular(int flag);
float getAngle(int flag);
float getQuat(int flag);
float getTemp();

int makeYawZero(void);
int updateIICAddr(std::string input);
#endif