// merged into decode_serial.cpp

#define SMALL_ENDIAN
// #define BIG_ENDIAN

#include "uart_slave/ControllerSerialProtocol.hpp"
#include "uart_slave/serial_receive.hpp"
#include <stdio.h>

/* parse the paylaod of uart_fd & feed into raw */
// static int parse_data(raw_t *raw)
// {
//     // get the Motor Voltage
//     raw->MotorVolt_L = REINTERPRET_AS_FLOAT(raw->C2Mbuf, BYTE_POS_C2M_LWHEELVOLTAGE);
//     raw->MotorVolt_R = REINTERPRET_AS_FLOAT(raw->C2Mbuf, BYTE_POS_C2M_RWHEELVOLTAGE);
//     return 1; // parsed successfully
// }

// int serial_input(raw_t *raw, uint8_t *Rx_buffer, const int num_bytes)
// {
//     // store the bytes into the raw struct
//     raw->C2Mbuf[raw->nbyte++] = Rx_buffer[0]; // store the first byte into the raw struct's buffer
//     for (int i = 1; i < num_bytes; i++){
//         raw->C2Mbuf[raw->nbyte++] = Rx_buffer[i];
//     }
    
//     // nbyte should be equal to num_bytes aka C2M_PACKET_SIZE
//     printf("nbyte = %d\n", raw->nbyte);

//     // check if the message is complete / corrupted
//     if (raw->C2Mbuf[raw->nbyte - 1] != END_BIT) // C2Mbuf[13]
//     {
//         printf("End bit error\n");
//         raw->nbyte = 0; // reset the nbyte
//         return -1;
//     }
//     if (raw->nbyte != C2M_PACKET_SIZE)
//     {
//         printf("Length error\n");
//         raw->nbyte = 0; // reset the nbyte
//         return -1;
//     }

//     // check the checksum
//     uint8_t checksum = 0;
//     for (int i = 0; i < BYTE_POS_C2M_CHECKSUM; i++)
//     { 
//         checksum += raw->C2Mbuf[i];
//     }
//     if (checksum != raw->C2Mbuf[BYTE_POS_C2M_CHECKSUM])
//     {
//         printf("Checksum error\n");
//         return -1;
//     }

//     raw->nbyte = 0; // reset the nbyte
//     // start parsing the data
//     return parse_data(raw);
// }