#include <stdio.h>
#include <stdlib.h>

#include "backend.h"

#define REGLOC 0x100
#define REGLEN 2 // 2 for ip

#define CHRPORT 0x300
#define SENDCHAR 0x310

unsigned short ip;
unsigned short cflags;
unsigned short ax;
unsigned short bx;
unsigned short cx;
unsigned short bp;
unsigned short sp;

#define $ readWord(ip, ram)

typedef struct doubleWord {
    unsigned short w1;
    unsigned short w2;
}doubleWord;

int tmpStorage;

void initRegisters(FILE *ram) {
    // Set the ip pointer to the register location
    ip = REGLOC;
    cflags = ip +2;
    ax = cflags + 1;
    bx = ax + 2;
    cx = bx + 2;

    bp = cx + 2;
    sp = bp + 2;
}

void reset(FILE *ram, FILE *disk) {
    // Find the reset vector at the end of disk
    unsigned short vector = readWord(DISKSIZE-2, disk);
    // Store it in the instruction pointer
    writeWord(vector, ip, ram);
}

void loadSegs(unsigned short diskSeg, unsigned short destAddr, int n, FILE *disk, FILE *ram) {
    // Loop over segments
    for (int i = 0; i < 512*n; i++)
    {
        // Get instruction
        byte diskInstr = readByte(diskSeg*512+i, disk);
        // Set instruction
        writeByte(diskInstr, destAddr+i, ram);
    }
}

void addToTmpStore(int *tmp, unsigned short data) {
    *tmp = *tmp << 16;
    *tmp += data;
}

void storeState(FILE *ram) {
    tmpStorage = 0;
    addToTmpStore(&tmpStorage, $);
}

unsigned short getRegValue(unsigned short reg, FILE *ram) {
    switch (reg)
    {
    case 0x01:  
        return ax;
        break;
    case 0x02:
        return bx;
        break;
    case 0x03:
        return cx;
        break;
    case 0x11:
        return readWord(ax, ram);
        break;
    case 0x12:
        return readWord(bx, ram);
        break;
    case 0x13:
        return readWord(cx, ram);
        break;
    }
}

unsigned short getReg(unsigned short rVal, FILE *ram) {
    switch (rVal)
    {
    case 0x01:  
        return ax;
        break;
    case 0x02:
        return bx;
        break;
    case 0x03:
        return cx;
        break;
    }
}

doubleWord getMemValue(unsigned short arg1, unsigned short arg2, unsigned short instr, FILE *ram) {
    doubleWord v;
    switch (instr>>8)
    {
    case 0x00:
        v.w1 = arg1;
        v.w2 = arg2;
        break;
    case 0x01:
        v.w1 = readWord(arg1, ram);
        v.w2 = arg2;
        break; 
    case 0x10:
        v.w1 = arg1;
        v.w2 = readWord(arg2, ram);
        break; 
    case 0x11:
        v.w1 = readWord(arg1, ram);
        v.w2 = readWord(arg2, ram);
        break; 
    }

    return v;
}

int handleInstr(unsigned short instr, unsigned short arg1, unsigned short arg2, FILE *ram, FILE *disk) {
    unsigned short tmpIp = (unsigned short)(tmpStorage%0xffff);
    unsigned short rv1 = getRegValue(arg1, ram);
    unsigned short rv2 = getRegValue(arg2, ram);
    doubleWord v = getMemValue(arg1, arg2, instr, ram);
    unsigned short v1 = v.w1;
    unsigned short v2 = v.w2;
    byte opcode = instr%256;

    unsigned short end;
    unsigned short reg;

    switch (opcode)
    {
    case 0x00:
        break;
    case 0x01:
        writeWord(v1, ip, ram);
    case 0x11:
        writeWord(rv1, ip, ram);

    case 0x02:
        writeWord(v1, v2, ram);
        break;
    case 0x12:
        writeWord(rv1, v2, ram);
        break;
    case 0x22:
        writeWord(v1, rv2, ram);
        break;
    case 0x32:
        writeWord(rv1, rv2, ram);
        break;

    case 0x33:
        end = readWord(rv1, ram) + readWord(rv2, ram);
        reg = getReg(rv1, ram);
        writeWord(end, reg, ram);
        break;
    
    case 0x34:
        end = readWord(rv1, ram) - readWord(rv2, ram);
        reg = getReg(rv1, ram);
        writeWord(end, reg, ram);
        break;
    case 0x10:
        return 1;
        break;
    default:
        return 1;
    }
    return 0;
}

int main() {
    FILE *ram;
    FILE *disk;
    ram = fopen("src/ram.bin", "wb+");

    // Fill ram with zeros
    byte *rambin = (byte*)malloc(sizeof(byte) * RAMSIZE);
    fwrite(rambin, RAMSIZE, 1, ram);
    free(rambin);

    disk = fopen("src/disk.bin", "rb+");
    /*byte *diskbin = (byte*)malloc(sizeof(byte) * DISKSIZE);
    fwrite(diskbin, DISKSIZE, 1, disk);
    free(diskbin);
    */

    initRegisters(ram);
    reset(ram, disk);

    // Load instructions into memory
    loadSegs(0, readWord(ip, ram), 1, disk, ram);

    unsigned short instr;
    int instrSize = 0;

    // Interpret Loop
    for (int i = 0; $ < RAMSIZE;i++) {
        // Get instruction
        instr = readWord($, ram);
        
        
        if (instr == 0x00) {
            instrSize = 0;
        } else if ((instr%16) == 0x01) {
            instrSize = 2;
        } else if ((instr%16) == 0x02) {
            instrSize = 4;
        } else if ((instr%16) == 0x03) {
            instrSize = 2;
        } else {
            instrSize = 0;
        }

        instrSize += 2;

        if (instr != 0) {
            //printf("%02x:%d\n", instr, instrSize);
        }

        unsigned short arg1 = readWord($+2, ram);
        unsigned short arg2 = readWord($+4, ram);

        if (instr != 0) {
            //printf("%04x    %02x %04x %04x\n", $, instr, arg1, arg2);
        }

        

        storeState(ram);
        // Inc ip to next instruction
        
        writeWord($+instrSize, ip, ram);
        if (handleInstr(instr, arg1, arg2, ram, disk)) {
            //printf("%04x    %02x %04x %04x\n", $, instr, arg1, arg2);
            break;
        };

        if (readWord(SENDCHAR, ram)) {
            printf("%c", readWord(CHRPORT, ram));
            writeWord(0x0, SENDCHAR, ram);
        }
    }

    fclose(ram);
    fclose(disk);
   
    return 0;
}