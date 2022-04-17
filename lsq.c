#include <stdio.h>
#include <stdlib.h>

#include "lsq.h"

void lsq_Entry(CPU_Stage APEX_RD2)
{
    
        if (lsq_head == - 1)
        {
            lsq_head = 0;
        }
        lsq_tail = (lsq_tail+1) % lsq_SIZE;
        lsq[lsq_tail].allocated = 0;
        lsq[lsq_tail].mem_address = 0;
        lsq[lsq_tail].pc_value = APEX_RD2.pc;

        if(APEX_RD2.lsBit == 0)
        {
            lsq[lsq_tail].prd = APEX_RD2.prd;
            lsq[lsq_tail].data_valid_Bit = 1;
            lsq[lsq_tail].lsBit = 0;
        }
        /*RS1 of Store*/
        else
        {
            lsq[lsq_tail].data_valid_Bit = APEX_RD2.prs1_valid;
            lsq[lsq_tail].prs1 = APEX_RD2.prs1;
            lsq[lsq_tail].prs1_value = APEX_RD2.rs1_value;
            lsq[lsq_tail].lsBit = 1;
        }
}

void lsq_to_mem()
{
    if (lsq_checkEmpty()) 
    {
        printf("lsq is empty\n");
    }
    else
    {
        if (lsq_head == lsq_tail) 
        {
            lsq_head = lsq_tail = -1;
        } 
        else 
        {
            //printf("\nInstruction issued for memory stage\n");
            lsq_head = (lsq_head + 1) % lsq_SIZE;
        }
    }
}

int lsq_checkFull()
{
    if ((lsq_head == lsq_tail + 1) || (lsq_head == 0 && lsq_tail == lsq_SIZE - 1))
    {
        printf("\nlsq is full\n");
        return 1;
    }
    return 0;
}

int lsq_checkEmpty()
{
    if (lsq_head == -1) 
    {
        return 1;
    }
  return 0;
}

void lsq_update(int prs1, int result)
{
    int i;
    
    printf("The elements of the queue are:\n");
    for (i = lsq_head; i != lsq_tail; i = (i + 1) % lsq_SIZE)
    {
        if(lsq[i].prs1 ==prs1)
        {
            lsq[i].prs1_value = result;
            lsq[i].data_valid_Bit = 1;
        }
    }
    if(lsq[i].prs1 ==prs1)
    {
        lsq[i].prs1_value = result;
        lsq[i].data_valid_Bit = 1;
    }
}