#define SMALL_ENDIAN
// #define BIG_ENDIAN

#include "uart_slave/MasterSerialProtocol.hpp"
#include "uart_slave/serial_receive.hpp"

#include <stdio.h>

/* parse the paylaod of uart_fd & feed into raw */
static int parse_data(raw_t *raw)
{
    // get the debug code
    raw->debug_code = raw->buf[BYTE_POS_S2M_DEBUGCODE];
    // get the FOC angle
    raw->foc_left = REINTERPRET_AS_FLOAT(raw->buf, BYTE_POS_S2M_LEFTFOCANGLE);
    raw->foc_right = REINTERPRET_AS_FLOAT(raw->buf, BYTE_POS_S2M_RIGHTFOCANGLE);

    return 1; // parsed successfully
}

static int decode(raw_t *raw)
{
    uint8_t checksum = 0;
    for (int i = 0; i < raw->len; i++){
        checksum += raw->buf[i];
    }
    if (checksum != raw->buf[raw->len]){
        printf("Checksum error\n");
        return -1;
    }
    // start parsing the data
    return parse_data(raw);
}

int serial_input(raw_t *raw, uint8_t* Rx_buffer, const int num_bytes)
{
    // message is not yet received, i.e. not yet find the start bit
    if (raw->nbyte == 0){
        // take the 1st byte
        raw->buf[0] = Rx_buffer[0];
        // check if there's any message
        if (raw->buf[0] != START_BIT){
            return 0; // no message
        }
        // have found the start bit
        raw->nbyte = 1;
        return 0;
    }

    // at here we should have already identified the start bit
    // now start input the data into the raw struct's buffer
    for (int i = 1; i < num_bytes; i++){
        raw->buf[raw->nbyte++] = Rx_buffer[i];
    }

    // check if the message is complete / corrupted
    if (raw->len != S2M_POCKET_SIZE){
        printf("Length error\n");
        raw->nbyte = 0; // reset the nbyte
        return -1;
    }

    if (raw->buf[raw->len] != END_BIT){
        printf("End bit error\n");
        raw->nbyte = 0; // reset the nbyte
        return -1;
    }

    // the message is complete, start decode
    raw->nbyte = 0; // reset the nbyte
    return decode(raw);
}