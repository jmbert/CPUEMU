#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../backend.h"

#define BUFFSIZE 1024

struct label{
    unsigned short addr;
    char *label;
}label;


char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( frontp != str && endp == frontp )
            *str = '\0';
    else if( str + len - 1 != endp )
            *(endp + 1) = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }

    return str;
}

int main() {
    FILE *src;
    char *buff = malloc(sizeof(char) * BUFFSIZE);
    struct label *labels = malloc(0);
    int numLabels;
    int origin;
    int *isAddr = malloc(sizeof(int)*5);

    FILE *disk;
    disk = fopen("blocks/disk.bin", "wb+");
    
    src = fopen("script.txt", "r");

    

    /*for (int line = 0;!feof(src);line++)
    {

        fgets(buff, BUFFSIZE, src);
        if (!strcmp(trim(buff), "")) {
            continue;
        }
        int size = strlen(buff);
        if (buff[size-1] == ':') {
            buff[size-1] = '\0';
            struct label l;
            l.label = buff;
            labels = (struct label*)realloc(labels, sizeof(labels) + sizeof(struct label));
            labels[numLabels] = l;
            numLabels++;
        }
        
    }

    fseek(src, 0, 1);*/

    for (int line = 0;!feof(src);line++)
    {
        int argNum = 0;

        fgets(buff, BUFFSIZE, src);
        if (!strcmp(trim(buff), "")) {
            continue;
        }
        char *instr = strtok(buff, " ");

        unsigned short bInstr;
        

        instr = trim(instr);

        if (!strcmp(instr, "jmp")) {
            bInstr = 0x01;
            isAddr[0] = 1;
            argNum = 1;
        } else if (!strcmp(instr, "org")) {
            unsigned short bArg;
            char *arg = strtok(NULL, ", ");

            bArg = strtol(arg, NULL, 0);

            int p = ftell(disk);
            
            writeWord(bArg, DISKSIZE-2, disk);

            fseek(disk, 0, p);
            origin = bArg;
            continue;
        } else if (!strcmp(instr, "nop")) {
            argNum = 0;
            bInstr = 0x0;
        } else if (!strcmp(instr, "mov")) {
            argNum = 2;
            isAddr[0] = 0;
            isAddr[1] = 1;
            bInstr = 0x02;
        } else if (!strcmp(instr, "add")) {
            argNum = 2;
            isAddr[0] = 0;
            isAddr[1] = 0;
            bInstr = 0x03;
        } else if (!strcmp(instr, "sub")) {
            
            argNum = 2;
            isAddr[0] = 0;
            isAddr[1] = 0;
            bInstr = 0x04;
        } else if (!strcmp(instr, "push")) {
            
            argNum = 1;
            isAddr[0] = 0;
            bInstr = 0x05;
        } else if (!strcmp(instr, "pop")) {
            argNum = 1;
            isAddr[0] = 0;
            bInstr = 0x06;
        } else if (!strcmp(instr, "end")) {
            argNum = 0;
            bInstr = 0xff;
        } else {
            break;
        }

        unsigned char *argBuffer = malloc(sizeof(unsigned short)*argNum);

        for (int i = 0; i < argNum; i++)
        {
            char *arg = strtok(NULL, ", ");
            arg = trim(arg);

            int addr = 0;
            if (arg[0] == '*') {    
                addr = 1;
                arg++;
            }

            unsigned short bArg;
            
            
            if (arg[0] == '%') {
                arg++;
                bInstr += 0x10*(i+1);
                if (!strcmp(arg, "ax")) {
                    bArg = 0x01;
                } else if (!strcmp(arg, "bx")) {
                    bArg = 0x02;
                } else if (!strcmp(arg, "cx")) {
                    bArg = 0x03;
                } else if (!strcmp(arg, "bp")) {
                    bArg = 0x04;
                } else if (!strcmp(arg, "sp")) {
                    bArg = 0x05;
                }
                if (addr) {
                    
                    bArg += 0x10;
                }
            } else if (!strcmp(arg, "$")) {
                bArg = ftell(disk)+origin;
            } else if (arg[0] == '\'') {
                arg++;
                bArg = arg[0];
            } else {
                bArg = strtol(arg, NULL, 0);
                if (addr) {
                    if (i == 0) {
                        bInstr += 0x0100;
                    } else {
                        bInstr += 0x1000;
                    }
                }
            }

            unsigned char b1 = bArg >> 8;
            unsigned char b2 = bArg % 256;
            argBuffer[i*2] = b1;
            argBuffer[i*2 + 1] = b2;
        }
        unsigned char b1 = bInstr>>8;
        unsigned char b2 = bInstr % 256;
        fwrite(&b1, 1, 1, disk);
        fwrite(&b2, 1, 1, disk);
        for (int i = 0; i < argNum*2; i++)
        {
            fwrite(&(argBuffer[i]), 1, 1, disk);
        }
        free(argBuffer);
    }

    free(labels);
    free(isAddr);
    free(buff);
    fclose(src);
    fclose(disk);
    
}