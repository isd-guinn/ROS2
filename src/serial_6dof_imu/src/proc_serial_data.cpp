// done porting
/*******************************************************************
 * Copyright:2016-2021 www.corvin.cn ROS小课堂
 * Description:使用串口方式读取和控制IMU模块信息.
 * Author: corvin
 * History:
 *   20211122:init this file.
******************************************************************/
#include <sstream>
#include <chrono>

// #include <ros/ros.h>
#include "rclcpp/rclcpp.hpp"
#include <stdio.hpp>
#include <stdlib.hpp>
#include <fcntl.hpp>
#include <unistd.hpp> // for write function
#include <termios.hpp>
#include <string.hpp>
#include <sys/time.hpp>
#include <sys/types.hpp>

#define  BYTE_CNT      55            //每次从串口中读取的总字节数
#define  ACCE_CONST    0.000488281   //用于计算加速度的常量16.0/32768.0
#define  ANGULAR_CONST 0.061035156   //用于计算角速度的常量2000.0/32768.0
#define  ANGLE_CONST   0.005493164   //用于计算欧拉角的常量180.0/32768.0

static unsigned char r_buf[BYTE_CNT];  //一次从串口中读取的数据存储缓冲区
static int port_fd = -1;  //串口打开时的文件描述符
static float acce[3],angular[3],angle[3],quater[4],temp;

/**************************************
 * Description:关闭串口文件描述符.
 *************************************/
int closeSerialPort(void)
{
    int ret = close(port_fd);
    return ret;
}

/*****************************************************************
 * Description:向IMU模块发送解锁命令,发送完命令需要延迟10ms.
 ****************************************************************/
static int send_unlockCmd(int fd)
{
    int ret = 0;
    unsigned char unLockCmd[5] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
    ret = write(fd, unLockCmd, sizeof(unLockCmd));
    if(ret != sizeof(unLockCmd))
    {
        // ROS_ERROR("Send IMU module unlock cmd error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "Send IMU module unlock cmd error !");
        return -1;
    }
    // ros::Duration(0.01).sleep(); //delay 10ms才能发送其他配置命令
    rclcpp::sleep_for(std::chrono::milliseconds(10));
    return 0;
}

/*****************************************************************
 * Description:向IMU模块发送保存命令,发送完命令需要延迟100ms.
 *****************************************************************/
static int send_saveCmd(int fd)
{
    int ret = 0;
    unsigned char saveCmd[5]   = {0xFF, 0xAA, 0x00, 0x00, 0x00};
    ret = write(fd, saveCmd, sizeof(saveCmd));
    if(ret != sizeof(saveCmd))
    {
        // ROS_ERROR("Send IMU module save cmd error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "Send IMU module save cmd error !");
        return -1;
    }
    // ros::Duration(0.1).sleep(); //delay 100ms才能发送其他配置命令
    rclcpp::sleep_for(std::chrono::milliseconds(100));
    return 0;
}

/*****************************************************************
 * Description:向IMU模块的串口中发送数据,第一步就是先解锁,其次才能
 *   发送控制命令,最后就是需要发送两遍解锁命令、保存命令的操作.
 *****************************************************************/
static int send_data(int fd, unsigned char *send_buffer, int length)
{
    int ret = 0;

    #if 0 //打印发送给IMU模块的数据,用于调试
    printf("Send %d byte to IMU:", length);
    for(int i=0; i<length; i++)
    {
        printf("0x%02x ", send_buffer[i]);
    }
    printf("\n");
    #endif
    send_unlockCmd(fd); //发送解锁命令

    //发送完控制命令,需要延时100ms
    ret = write(fd, send_buffer, length*sizeof(unsigned char));
    if(ret != length)
    {
        // ROS_ERROR("Send IMU module control cmd error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "Send IMU module control cmd error !");
        return -2;
    }
    // ros::Duration(0.1).sleep(); //delay 100ms才能发送其他配置命令
    rclcpp::sleep_for(std::chrono::milliseconds(100));

    send_saveCmd(fd);  //发送保存命令
    return 0;
}

/**************************************************************
 * Description:根据串口数据协议来解析有效数据,这里共有4种数据.
 *   解析读取其中的44个字节,正好是4帧数据,每一帧数据都是11个字节.
 *   有效数据包括加速度输出(0x55 0x51),角速度输出(0x55 0x52)
 *   角度输出(0x55 0x53),四元素输出(0x55 0x59).
 *************************************************************/
static void parse_serialData(unsigned char chr)
{
    static unsigned char chrBuf[BYTE_CNT];
    static unsigned char chrCnt = 0;  //记录读取的第几个字节

    signed short sData[4]; //save 8 Byte有效信息
    unsigned char i = 0;   //用于for循环
    unsigned char frameSum = 0;  //存储数据帧的校验和

    chrBuf[chrCnt++] = chr; //保存当前字节,字节计数加1

    //判断是否读取满一个完整数据帧11个字节,若没有则返回不进行解析
    if(chrCnt < 11)
        return;

    //读取满完整一帧数据,计算数据帧的前十个字节的校验和,累加即可得到
    for(i=0; i<10; i++)
    {
        frameSum += chrBuf[i];
    }

    //找到数据帧第一个字节是0x55,同时判断校验和是否正确,若两者有一个不正确,
    //都需要移动到最开始的字节,再读取新的字节进行判断数据帧完整性
    if ((chrBuf[0] != 0x55)||(frameSum != chrBuf[10]))
    {
        memcpy(&chrBuf[0], &chrBuf[1], 10); //将有效数据往前移动1字节位置
        chrCnt--; //字节计数减1,需要再多读取一个字节进来,重新判断数据帧
        return;
    }

    #if 0 //打印出完整的带帧头尾的数据帧
    for(i=0; i<11; i++)
      printf("0x%02x ",chrBuf[i]);
    printf("\n");
    #endif

    memcpy(&sData[0], &chrBuf[2], 8);
    switch(chrBuf[1])  //根据数据帧不同类型来进行解析数据
    {
        case 0x51: //x,y,z轴 加速度输出
            acce[0] = ((short)(((short)chrBuf[3]<<8)|chrBuf[2]))*ACCE_CONST;
            acce[1] = ((short)(((short)chrBuf[5]<<8)|chrBuf[4]))*ACCE_CONST;
            acce[2] = ((short)(((short)chrBuf[7]<<8)|chrBuf[6]))*ACCE_CONST;
            break;
        case 0x52: //角速度输出
            angular[0] = ((short)(((short)chrBuf[3]<<8)|chrBuf[2]))*ANGULAR_CONST;
            angular[1] = ((short)(((short)chrBuf[5]<<8)|chrBuf[4]))*ANGULAR_CONST;
            angular[2] = ((short)(((short)chrBuf[7]<<8)|chrBuf[6]))*ANGULAR_CONST;
            temp       = ((float)(((short)chrBuf[9]<<8)|chrBuf[8]))/100.0;
            break;
        case 0x53: //欧拉角度输出, roll, pitch, yaw
            angle[0] = ((short)(((short)chrBuf[3]<<8)|chrBuf[2]))*ANGLE_CONST;
            angle[1] = ((short)(((short)chrBuf[5]<<8)|chrBuf[4]))*ANGLE_CONST;
            angle[2] = ((short)(((short)chrBuf[7]<<8)|chrBuf[6]))*ANGLE_CONST;
            break;
        case 0x59: //四元素输出
            quater[0] = ((short)(((short)chrBuf[3]<<8)|chrBuf[2]))/32768.0;
            quater[1] = ((short)(((short)chrBuf[5]<<8)|chrBuf[4]))/32768.0;
            quater[2] = ((short)(((short)chrBuf[7]<<8)|chrBuf[6]))/32768.0;
            quater[3] = ((short)(((short)chrBuf[9]<<8)|chrBuf[8]))/32768.0;
            break;
        default:
            // ROS_ERROR("parse imu data frame type error !");
            RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "parse imu data frame type error !");
            break;
    }
    chrCnt = 0;
}

/********************************************************************
 * Description:将yaw角度归零,这里需要通过串口发送的命令如下所示,
 *  发送z轴角度归零的固定命令:0xFF 0xAA 0x01 0x04 0x00;
 *******************************************************************/
int makeYawZero(void)
{
    int ret = 0;
    unsigned char yawZeroCmd[5] = {0xFF, 0xAA, 0x01, 0x04, 0x00};

    ret = send_data(port_fd, yawZeroCmd, sizeof(yawZeroCmd));
    if(ret != 0) //通过串口发送命令失败
    {
        // ROS_ERROR("Send yaw zero control cmd error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "Send yaw zero control cmd error !");
        return -1;
    }
    // ROS_WARN("Set yaw to zero radian successfully !");
    RCLCPP_WARN(rclcpp::get_logger("imu_data_pub_node"), "Set yaw to zero radian successfully !");

    return 0;
}

/*******************************************************************
 * Description:更新IMU模块的IIC地址,这里通过发送串口命令方式来更新IIC地址,
 *   完整的控制命令为:0xFF 0xAA 0x1A 0x** 0x00,更新的IIC地址为第4个字节.
 ******************************************************************/
int updateIICAddr(std::string input)
{
    int ret = 0;
    std::stringstream num;
    int addr = 0;
    const char *iicAddr = NULL;
    unsigned char updateIICAddrCmd[5] = {0xFF, 0xAA, 0x1A, 0x00, 0x00};
    
    iicAddr = input.c_str();
    num << std::hex << iicAddr+2 << std::endl;
    num >> addr;
    updateIICAddrCmd[3] = addr;

    #if 0
    for (int i=0; i<5; i++)
    {
        printf("0x%02x ", updateIICAddrCmd[i]);
    }
    printf("\n\n");
    #endif
 
    ret = send_data(port_fd, updateIICAddrCmd, sizeof(updateIICAddrCmd));
    if(ret != 0)
    {
        // ROS_ERROR("Update IMU Module IIC Address Error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "Update IMU Module IIC Address Error !");
        return -1;
    }
    // ROS_WARN("Update IMU Module IIC Address [%s] successfully !", input.c_str());
    RCLCPP_WARN(rclcpp::get_logger("imu_data_pub_node"), "Update IMU Module IIC Address [%s] successfully !", input.c_str());

    return 0;
}

/*****************************************************************
 * Description:根据串口配置信息来连接IMU串口,准备进行数据通信,
 *   输入参数的意义如下:
 *   const char* path:IMU设备的挂载地址;
 ****************************************************************/
int initSerialPort(const char* path)
{
    struct termios terminfo;
    bzero(&terminfo, sizeof(terminfo));

    port_fd = open(path, O_RDWR|O_NOCTTY);
    if (-1 == port_fd)
    {
        // ROS_ERROR("Open 6DOF IMU dev error:%s", path);
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "Open 6DOF IMU dev error:%s", path);
        return -1;
    }

    //判断当前连接的设备是否为终端设备
    if (isatty(STDIN_FILENO) == 0)
    {
        // ROS_ERROR("IMU dev isatty error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "IMU dev isatty error !");
        return -2;
    }

    /*设置串口通信波特率-115200bps*/
    cfsetispeed(&terminfo, B115200);
    cfsetospeed(&terminfo, B115200);

    //设置数据位 - 8 bit
    terminfo.c_cflag |= CLOCAL|CREAD;
    terminfo.c_cflag &= ~CSIZE;
    terminfo.c_cflag |= CS8;

    //不设置奇偶校验位 - N
    terminfo.c_cflag &= ~PARENB;

    //设置停止位 - 1
    terminfo.c_cflag &= ~CSTOPB;

    //设置等待时间和最小接收字符
    terminfo.c_cc[VTIME] = 0;
    terminfo.c_cc[VMIN]  = 1;

    //清除串口缓存的数据
    tcflush(port_fd, TCIFLUSH);

    if((tcsetattr(port_fd, TCSANOW, &terminfo)) != 0)
    {
        // ROS_ERROR("set imu serial port attr error !");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "set imu serial port attr error !");
        return -3;
    }

    return 0;
}

/******************************************
 * Description:得到三轴加速度信息,输入参数可以
 * 　为0,1,2分别代表x,y,z轴的加速度信息.
 *****************************************/
float getAcc(int flag)
{
    return acce[flag];
}

/******************************************
 * Description:得到角速度信息,输入参数可以是
 * 　0,1,2,分别代表x,y,z三轴的角速度信息.
 *****************************************/
float getAngular(int flag)
{
    return angular[flag];
}

/********************************************
 * Description:获取yaw,pitch,roll,输入参数0,
 * 1,2可以分别获取到roll,pitch,yaw的数据.
 *******************************************/
float getAngle(int flag)
{
    return angle[flag];
}

/********************************************
 * Description:输入参数0,1,2,3可以分别获取到
 * 　四元素的q0,q1,q2,q3.但是这里对应的ros中
 * 　的imu_msg.orientation.w,x,y,z的顺序,
 * 　这里的q0是对应w参数,1,2,3对应x,y,z.
 ********************************************/
float getQuat(int flag)
{
    return quater[flag];
}

/********************************************
 * Description:返回IMU芯片内部实时温度值.
 ********************************************/
float getTemp(void)
{
    return temp;
}

/*************************************************************
 * Description:从串口读取数据,然后解析出各有效数据段,这里
 *  一次性从串口中读取88个字节,然后需要从这些字节中进行
 *  解析读取44个字节,正好是4帧数据,每一帧数据是11个字节.
 *  有效数据包括加速度输出(0x55 0x51),角速度输出(0x55 0x52)
 *  角度输出(0x55 0x53),四元素输出(0x55 0x59).
 *************************************************************/
int getImuData(void)
{
    int data_len = 0;
    bzero(r_buf, sizeof(r_buf)); //存储新数据前,将缓冲区清零

    //开始从串口中读取数据,并将其保存到r_buf缓冲区中
    data_len = read(port_fd, r_buf, sizeof(r_buf));
    if(data_len <= 0) //从串口中读取数据失败
    {
        // ROS_ERROR("read serial port data failed !\n");
        RCLCPP_ERROR(rclcpp::get_logger("imu_data_pub_node"), "read serial port data failed !\n");
        closeSerialPort();
        return -1;
    }

    //printf("recv data:%d byte\n", data_len); //一次性从串口中读取的总数据字节数
    for (int i=0; i<data_len; i++) //将读取到的数据进行解析
    {
        //printf("0x%02x ", r_buf[i]);
        parse_serialData(r_buf[i]);
    }
    //printf("\n\n");

    return 0;
}