#include <stdio.h>

#define byte unsigned char

#define RAMSIZE 0x1000
#define DISKSIZE 0x100

byte readByte(unsigned short addr, FILE *block);
void writeByte(byte data, unsigned short addr, FILE *block);

unsigned short readWord(unsigned short addr, FILE *block);
void writeWord(unsigned short data, unsigned short addr, FILE *block);