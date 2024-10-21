#include <iostream>
#include <cstdint>

#define REINTERPRET_AS_FLOAT(x,y) (*(float*)(&x[y]))
#define EXTRACT_BYTE_FROM_4BYTE_VALUE(x,y) (*((uint8_t*)(&x)+y)) // small endian

int main() {
    float myFloat = 123.0f;
    uint8_t byteArray[6];

    // Extract each byte from the float
    for (int i = 0; i < 4; ++i) {
        byteArray[i] = EXTRACT_BYTE_FROM_4BYTE_VALUE(myFloat, i);
    }

    // Print the bytes
    std::cout << "Bytes of the float: ";
    for (int i = 0; i < 4; ++i) {
        std::cout << std::hex << static_cast<int>(byteArray[i]) << " ";
    }
    std::cout << std::endl;

    // Reinterpret the bytes as a float
    // 0 = start bit of the byte array
    float newFloat = REINTERPRET_AS_FLOAT(byteArray, 0);

    // print the new float
    std::cout << "Reinterpreted float: " << newFloat << std::endl;

    return 0;
}