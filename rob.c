#include <stdio.h>
#include <stdlib.h>

#include "rob.h"

void rob_Entry(CPU_Stage APEX_RD2)
{
    
        if (rob_head == - 1)
        {
            rob_head = 0;
        }

        rob_tail = (rob_tail+1) % rob_SIZE;
        rob[rob_tail].allocated = 0; 
        rob[rob_tail].itype = 0; 
        strcpy(rob[rob_tail].opcode_str, APEX_RD2.opcode_str);
        rob[rob_tail].opcode = APEX_RD2.opcode;
        rob[rob_tail].pc_value = APEX_RD2.pc;
        rob[rob_tail].rd = APEX_RD2.rd; //architectural register address
        rob[rob_tail].prd = APEX_RD2.prd; //physical register address
        rob[rob_tail].excodes = 0; // exception codes
        rob[rob_tail].lsBit = APEX_RD2.lsBit;
        rob[rob_tail].lsqindex = APEX_RD2.lsqIndex; // exception codes
}

void rob_Commitment()
{
    if (rob_checkEmpty()) 
    {
        printf("ROB is empty\n");
        //return -1;
    }
    else
    {
        if (rob_head == rob_tail) 
        {
            rob_head = rob_tail = -1;
        } 
        else 
        {
            printf("\nInstruction issued for writeback\n");
            rob_head = (rob_head + 1) % rob_SIZE;
        }
    }
}

int rob_checkFull()
{
    if ((rob_head == rob_tail + 1) || (rob_head == 0 && rob_tail == rob_SIZE - 1))
    {
        printf("ROB is full\n");
        return 1;
    }
    return 0;
}

int rob_checkEmpty()
{
    if (rob_head == -1) 
    {
        return 1;
    }
  return 0;
}

void rob_update(int pc, int result)
{
    int i;
    
        //printf("The elements of the queue are:\n");
        for (i = rob_head; i != rob_tail; i = (i + 1) % rob_SIZE)
        {
            if(rob[i].pc_value ==pc)
            {
                rob[i].result = result;
                rob[i].allocated = 1;
            }
        }
        if(rob[rob_tail].pc_value ==pc)
        {
            rob[rob_tail].result = result;
            rob[rob_tail].allocated = 1;
        }
}