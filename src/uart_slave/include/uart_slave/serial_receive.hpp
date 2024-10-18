#ifndef SERIAL_RECEIVE_HPP 
#define SERIAL_RECEIVE_HPP

#include <stdint.h>
#include <MasterSerialProtocol.hpp>

typedef struct
{
    int nbyte;                          /* number of bytes in message buffer */ 
    int len;                            /* message length (bytes) */
    uint8_t buf[S2M_POCKET_SIZE];       /* message raw buffer */
    
    uint8_t debug_code;                    /* debug code */
    float foc_left;                     /* left foc angle */
    float foc_right;                    /* right foc angle */
}raw_t;

/* Get raw data from serial port 
* args:     raw_t *raw      IO  receiver raw data control struct -> raw will be updated
*           uint8_t data    I   stream data (1 byte)
* return:   status (-1: error message, 0: no message, 1: input data successfully)
*/
int serial_input(raw_t *raw, uint8_t data);

#endif