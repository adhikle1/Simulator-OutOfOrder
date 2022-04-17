#ifndef _ISSUE_QUEUE_H_
#define _ISSUE_QUEUE_H_

#include "apex_macros.h"
#include "apex_cpu.h"
//#include "apex_cpu.c"

//void checkIQhasEntry();


int curr_iqSize;

typedef struct IQdata
{
    int allocatedBit;  /*allocatedBit Bit*/
    int pc;
    char opcode_str[128];
    int opcode;
    int fuType; /*Type of FU 0 -> IU, 1 -> MU, 2 -> BU*/
    int imm; /*Literal value*/
    int src1Ready; /*Source 1 Ready Bit*/
    int prs1;   /*Source 1 Tag*/
    int prs1Value;  /*Source 1 Value*/
    int src2Ready; /*Source 1 Ready Bit*/
    int prs2;   /*Source 1 Tag*/
    int prs2Value;  /*Source 1 Value*/
    int dest; /*Destination Physical Register*/
    int lsqIndex; /*LSQ Index*/
    struct IQdata *next;
}IQdata; 

typedef struct IQdata *iqPtr;
iqPtr iqPtr_head;

int is_IQ_Empty();
int is_IQ_Full(int curr_iqSize);
void IQupdate();
void iqEntry();
void flush_IQ();
IQdata *issue_Instruction(int fu);

#endif