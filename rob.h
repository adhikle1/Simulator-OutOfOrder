#ifndef _ROB_H_
#define _ROB_H_

#include "apex_macros.h"
#include "apex_cpu.h"
//#include "apex_cpu.c"

typedef struct ROBData {
    int allocated;  //indicating if the result/address is valid  (0 - invalid and 1 - valid)
    int itype; //0 for Arithmatic instructions 1 for Load/Store
    char opcode_str[128];
    int opcode;
    int pc_value; //pc value
    int rd; //architectural register address
    int prd; //physical register address
    int excodes; // exception codes
    int mready; //to specify if it's ready for memory operations 0 - not ready and 1 - ready
    int svalue; // value of register to be stored to memory, used by STORE
    int sval_valid; //used by store instructions, its valid bit
    int result; //result of the instruction
    int lsBit;
    int lsqindex;
} ROBData;

struct ROBData *rob;

int rob_head, rob_tail; // To rob_Entry and rob_Commitment the data elements

void rob_Entry(); // Function used to put instructions in rob
void rob_Commitment(); // Function used to commit instructions from rob
void rob_update();     //to update result
int rob_checkFull();    //Check if ROB is full
int rob_checkEmpty(); //Check if ROB is empty

#endif