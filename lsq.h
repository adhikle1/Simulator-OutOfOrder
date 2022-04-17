#ifndef _lsq_H_
#define _lsq_H_

#include "apex_macros.h"
#include "apex_cpu.h"

typedef struct LSQData {
    int allocated;  //indicating if entry is free
    int lsBit; //0 Load 1 Store
    int mem_address_vBit; //(
    int mem_address;    //Calculayed by IU
    int prd; //for load
    int data_valid_Bit; //(STORE) - Source 1
    int prs1; //source where value is to be stored (STORE)
    int prs1_value; // calue to be stored (STORE)
    int pc_value;
} LSQData;

struct LSQData *lsq;

int lsq_head, lsq_tail; // To lsq_Entry and lsq_Commitment the data elements

void lsq_Entry(); // Function used to put instructions in lsq
void lsq_to_mem(); // Function used to commit instructions from lsq
void lsq_update();     //to update result
int lsq_checkFull();    //Check if lsq is full
int lsq_checkEmpty(); //Check if lsq is empty

#endif