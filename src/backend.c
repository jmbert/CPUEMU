#include "backend.h"

byte readByte(unsigned short addr, FILE *block) {
    byte data;
    // Jump to address
    fseek(block, addr, 0);
    // Read byte
    fread(&data, 1, 1, block);
    return data;
}

unsigned short readWord(unsigned short addr, FILE *block) {
    unsigned short data;
    // Read bytes
    byte b1 = readByte(addr, block);
    byte b2 = readByte(addr+1, block);
    // Shift first byte 1 byte left, then add second byte
    data = ((unsigned short)b1 << 8 )+ b2;

    return data;
}

void writeByte(byte data, unsigned short addr, FILE *block) {
    // Jump to address
    fseek(block, addr, 0);
    // Write byte
    fwrite(&data, 1, 1, block);
}

void writeWord(unsigned short data, unsigned short addr, FILE *block) {
    // Get both bytes of data
    byte b1 = data >> 8;
    byte b2 = data%256;
    // Write bytes to block
    writeByte(b1, addr, block);
    writeByte(b2, addr+1, block);
}