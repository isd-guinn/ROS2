#ifndef SERIALCOMMUNICATION_HPP
#define SERIALCOMMUNICATION_HPP

/*  SerialCommunation Protocol
    For RPI to MCU   */
/*  */
/*  
    Pocket:   { 0x3E  0x00  0x00  0x00  0x00  }
              {  '>'   ' '   ' '   ' '   ' '  }

    |     1st Bit     |     2nd Bit     |     3rd Bit     |     4th Bit     |     5th Bit     |
    |     StartBit    |     EStopBit    |     WBDirBit    |     WBPwrBit    |     CheckSum    |
*/

#define POCKET_SIZE 4

// Define the Bit for the Protocol
#define StartBit 0x3E

// Wheelbase Enable Bits
#define EStopEnabledBit  0xA1
#define EstopDisabledBit 0xA2

// Wheelbase Direction Bits
#define WBDirStopBit  0xA0
#define WBDirForwardBit  0xA1
#define WBDirBackwardBit 0xA2
#define WBDirClockwiseBit 0xA3
#define WBDirAntiClkwiseBit 0xA4

#endif