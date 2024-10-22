#ifndef SERIAL_RECEIVE_HPP 
#define SERIAL_RECEIVE_HPP

#include <stdint.h>
#include "uart_slave/MasterSerialProtocol.hpp"
#include "uart_slave/ControllerSerialProtocol.hpp"

// typedef struct
// {
//     int nbyte;                          /* number of bytes in message buffer */ 
//     uint8_t S2Mbuf[S2M_PACKET_SIZE];       /* message raw buffer */
//     uint8_t C2Mbuf[C2M_PACKET_SIZE];       /* message raw buffer */
    
//     uint8_t debug_code;                    /* debug code */
//     float foc_left;                     /* left foc angle */
//     float foc_right;                    /* right foc angle */

//     float MotorVolt_L;                     /* left foc angle */
//     float MotorVolt_R;                    /* right foc angle */
// }raw_t;

typedef struct
{
    int nbyte;                          /* number of bytes in message buffer */ 
    uint8_t buf[C2M_PACKET_SIZE];       /* message raw buffer */

    float MotorVolt_L;                     /* left foc angle */
    float MotorVolt_R;                    /* right foc angle */
} C2Mraw_t;

typedef struct
{
    int nbyte;                          /* number of bytes in message buffer */ 
    uint8_t buf[S2M_PACKET_SIZE];       /* message raw buffer */

    uint8_t debug_code;                 /* debug code */
    float foc_left;                     /* left foc angle */
    float foc_right;                    /* right foc angle */

} S2Mraw_t;

/* Get raw data from serial port 
* args:     raw_t *raw              IO  receiver raw data control struct -> raw will be updated
*           uint8_t *Rx_buffer      I   stream data (1 byte)
*           int num_bytes            I   number of bytes received    
* return:   status (-1: error message, 0: no message, 1: input data successfully)
*/
// int serial_input(raw_t *raw, uint8_t* Rx_buffer, const int num_bytes);
int serial_input(S2Mraw_t *raw, uint8_t *Rx_buffer, const int num_bytes);
int serial_input(C2Mraw_t *raw, uint8_t *Rx_buffer, const int num_bytes);
#endif