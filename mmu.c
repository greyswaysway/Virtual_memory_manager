#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "tlb.h"
#include "pgtable.h" //Only used for the 128 bit 

#define PAGE_SIZE  256
#define PAGES1  256
#define PAGES2  128
#define BUFFER_SIZE 10
#define MEMSIZE PAGE_SIZE * PAGES1
#define MEMSIZE2 PAGE_SIZE * PAGES2

int main(int argc, char *argv[])
{
    signed char *storagePtr;
    char temp[BUFFER_SIZE];
    int pgfault = 0;
    int tlbHit = 0;
    int totalAddress = 0;
    int nextPg = 0;
    char *rate = argv[1];
    char *storeFile = argv[2];
    char *inputFile = argv[3];
    int storeFd = open(storeFile, O_RDONLY);
    FILE *in = fopen(inputFile, "r");
    if (in == NULL)
    {
        printf("Unable to open file.\n");
    }
    if (strcmp(rate, "256") == 0)
    {
        signed char mainMem[MEMSIZE];
        storagePtr = mmap(0, MEMSIZE, PROT_READ, MAP_PRIVATE, storeFd, 0);
        FILE *out = fopen("output256.csv", "w");
        int pagetable[PAGES1];
        struct tlb tlbTable[16];
        for(int x = 0; x < PAGES1; x++){
            pagetable[x] = -256;
        }
        for(int x = 0; x < 16; x++){
            struct tlb dummy;
            dummy.pageNum = -256;
            dummy.value = 0;
            tlbTable[x] = dummy;
        }
        while(fgets(temp, BUFFER_SIZE, in) != NULL) 
        {
            int realNumber;
            int rem = 1;
            int block = 1;
            int block2 = 1;
            int pgNumBin = 0;
            int offsetBin = 0;
            int pgNum = 0;
            int offset = 0;
            sscanf(temp, "%d", &realNumber);
            int x = 0;
            fprintf(out, "%d,", realNumber);
            while(realNumber != 0){
                rem = realNumber % 2;
                realNumber /= 2;
                if (x < 8){
                    offsetBin += rem * block;
                    block *= 10;
                }
                else if(x < 16){
                    pgNumBin += rem * block2;
                    block2 *= 10;
                }
                x++;
            }
            block = 1;
            while(offsetBin != 0){
                offset += (offsetBin % 10)  * block;
                offsetBin /=10;
                block *= 2;
            }
            block = 1;
            while(pgNumBin != 0){
                pgNum += (pgNumBin % 10)  * block;
                pgNumBin /= 10;
                block *= 2;
            }
            int flag = 0;
            int realAddr = 0;
            for (int x = 0; x < 16; x++){
                if(tlbTable[x].pageNum == pgNum){
                    realAddr = tlbTable[x].value;
                    tlbHit++;
                    flag = 1;
                }
            }
            if(flag == 0){
                realAddr = pagetable[pgNum];
                if(realAddr != -256){
                    struct tlb newTLB;
                    newTLB.pageNum = pgNum;
                    newTLB.value = realAddr;
                    for(int x = 0; x < 15; x++){
                        tlbTable[x] = tlbTable[x + 1];
                    }
                    tlbTable[15] = newTLB;
                }
            }
            if(realAddr == -256){
                pgfault++;
                realAddr = nextPg;
                nextPg++;
                memcpy(mainMem + realAddr * PAGE_SIZE, storagePtr + pgNum * PAGE_SIZE, PAGE_SIZE);
                pagetable[pgNum] = realAddr;
                struct tlb newTLB;
                newTLB.pageNum = pgNum;
                newTLB.value = realAddr;
                for(int x = 0; x < 15; x++){
                    tlbTable[x] = tlbTable[x + 1];
                }
                tlbTable[15] = newTLB;
            }
            signed char value = mainMem[realAddr * PAGE_SIZE + offset];
            int shift = 1;
            for(int x = 0; x < 8; x++){
                shift *= 2;
            }
            realAddr *= shift;
            int addr = realAddr | offset;
            fprintf(out, "%d,", addr);
            fprintf(out, "%d\n", value);
            totalAddress++;
        }
        fprintf(out, "Page Faults Rate, %.2f%%,\n", (pgfault / (totalAddress * 1.0)) * 100);
        fprintf(out, "TLB Hits Rate, %.2f%%,", (tlbHit / (totalAddress * 1.0)) * 100);
    }
    else if (strcmp(rate, "128") == 0)//This will execute for the rate of 128
    {
        signed char mainMem[MEMSIZE2];
        storagePtr = mmap(0, MEMSIZE, PROT_READ, MAP_PRIVATE, storeFd, 0);
        FILE *out = fopen("output128.csv", "w");
        struct pgEntry pagetable[PAGES2];
        struct tlb tlbTable[16];
        int timeCounter = 0;
        for(int x = 0; x < PAGES2; x++){
            struct pgEntry tempEntry;
            tempEntry.pageNum = -256;
            tempEntry.value = -256;
            tempEntry.lastTimeUsed = -256;
            pagetable[x] = tempEntry;
        }
        for(int x = 0; x < 16; x++){
            struct tlb dummy;
            dummy.pageNum = -256;
            dummy.value = -256;
            tlbTable[x] = dummy;
        }
        while(fgets(temp, BUFFER_SIZE, in) != NULL) 
        {
            timeCounter++;
            int realNumber;
            int rem = 1;
            int block = 1;
            int block2 = 1;
            int pgNumBin = 0;
            int offsetBin = 0;
            int pgNum = 0;
            int offset = 0;
            sscanf(temp, "%d", &realNumber);
            int i = 0;
            fprintf(out, "%d,", realNumber);
            while(realNumber != 0){
                rem = realNumber % 2;
                realNumber /= 2;
                if (i < 8){
                    offsetBin += rem * block;
                    block *= 10;
                }
                else if(i < 16){
                    pgNumBin += rem * block2;
                    block2 *= 10;
                }
                i++;
            }
            block = 1;
            while(offsetBin != 0){
                offset += (offsetBin % 10)  * block;
                offsetBin /=10;
                block *= 2;
            }
            block = 1;
            while(pgNumBin != 0){
                pgNum += (pgNumBin % 10)  * block;
                pgNumBin /= 10;
                block *= 2;
            }
            int flag = 0;
            int realAddr = 0;
            for (int x = 0; x < 16; x++){
                if(tlbTable[x].pageNum == pgNum){
                    realAddr = tlbTable[x].value;
                    tlbHit++;
                    flag = 1;
                    for(int y = 0; y < PAGES2; y++){
                        if(pagetable[y].pageNum == pgNum){
                            pagetable[y].lastTimeUsed = timeCounter;
                        }
                    }
                }
            }
            if(flag == 0){
                for(int x = 0; x < PAGES2; x++){
                    if(pagetable[x].pageNum == pgNum){
                        realAddr = pagetable[x].value;
                        pagetable[x].lastTimeUsed = timeCounter;
                        struct tlb newTLB;
                        newTLB.pageNum = pgNum;
                        newTLB.value = realAddr;
                        for(int y = 0; y < 15; y++){
                            tlbTable[y] = tlbTable[y + 1];
                        }
                        tlbTable[15] = newTLB;
                        flag = 1;
                    }
                }
            }
            if(flag == 0){
                pgfault++;
                int last = 0;
                if(nextPg < 128){
                    realAddr = nextPg;
                    last = nextPg;
                    nextPg++;
                }
                else{
                    int lastTime = timeCounter;
                    for(int x = 0; x < PAGES2; x++){
                        if(lastTime > pagetable[x].lastTimeUsed){
                            last = x;
                            lastTime = pagetable[x].lastTimeUsed;
                        }
                    }
                    realAddr = pagetable[last].value;
                }
                memcpy(mainMem + realAddr * PAGE_SIZE, storagePtr + pgNum * PAGE_SIZE, PAGE_SIZE);
                struct pgEntry tempEntry;
                tempEntry.pageNum = pgNum;
                tempEntry.value = realAddr;
                tempEntry.lastTimeUsed = timeCounter;
                pagetable[last] = tempEntry;
                struct tlb newTLB;
                newTLB.pageNum = pgNum;
                newTLB.value = realAddr;
                for(int y = 0; y < 15; y++){
                    tlbTable[y] = tlbTable[y + 1];
                }
                tlbTable[15] = newTLB;
            }
            signed char value = mainMem[realAddr * PAGE_SIZE + offset];
            int shift = 1;
            for(int x = 0; x < 8; x++){
                shift *= 2;
            }
            realAddr *= shift;
            int addr = realAddr | offset;
            fprintf(out, "%d,", addr);
            fprintf(out, "%d\n", value);
            totalAddress++;
        }
        fprintf(out, "Page Faults Rate, %.2f%%,\n", (pgfault / (totalAddress * 1.0)) * 100);
        fprintf(out, "TLB Hits Rate, %.2f%%,", (tlbHit / (totalAddress * 1.0)) * 100);
    }
    fclose(in);

    return 0;
}