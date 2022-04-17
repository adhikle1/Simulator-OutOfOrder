#include<stdlib.h>
#include <stdio.h>

#include "issue_queue.h"

int is_IQ_Empty() 
{
    return iqPtr_head == NULL;
}

int is_IQ_Full(int curr_iqSize)
{
    //printf("Current IQ size%d\n", curr_iqSize);
    if((curr_iqSize + 1) <= (iq_SIZE-1))
        return 0;
    return 1;
}

/*
void checkIQhasEntry()
{
    int temp = iq_SIZE-curr_iqSize;
    printf("No of IQ entries available: %d \n",temp);
}
*/
void IQupdate(int prd, int result)
{
    iqPtr IQ_currPtr = iqPtr_head;
    while (IQ_currPtr != NULL) 
    {
        if(IQ_currPtr->prs1 ==prd)
        {
            IQ_currPtr->prs1Value = result;
            IQ_currPtr->src1Ready = 1;
        }
        if(IQ_currPtr->prs2 ==prd)
        {
            IQ_currPtr->prs2Value = result;
            IQ_currPtr->src2Ready = 1;
        }
        IQ_currPtr = IQ_currPtr->next;
    }  
}

void iqEntry(CPU_Stage APEX_RD2)
{
    
    iqPtr iq = malloc(sizeof(IQdata));
    curr_iqSize++;

    iq->allocatedBit = 0;
    iq->pc = APEX_RD2.pc;
    strcpy(iq->opcode_str, APEX_RD2.opcode_str);
    iq->opcode = APEX_RD2.opcode;
    iq->fuType = APEX_RD2.fuType;
    iq->imm = APEX_RD2.imm; /*Literal value*/
    iq->src1Ready = 1; /*Source 1 Ready Bit*/
    iq->prs1 = APEX_RD2.prs1;   /*Source 1 Tag*/
    iq->prs1Value = APEX_RD2.rs1_value;  /*Source 1 Value*/
    iq->src2Ready = 1; /*Source 1 Ready Bit*/
    iq->prs2 = APEX_RD2.prs2;   /*Source 2 Tag*/
    iq->prs2Value = APEX_RD2.rs2_value;  /*Source 2 Value*/
    iq->dest = APEX_RD2.prd; /*Destination Physical Register*/
    iq->lsqIndex = APEX_RD2.lsqIndex; /*LSQ index*/
    //int cycleNo;
    
    iq->next = NULL;
    
    if (is_IQ_Empty()) 
    {
        iqPtr_head = iq;
    } 
    else 
    {
        // find the last node 
        iqPtr IQ_currPtr = iqPtr_head;
        while (IQ_currPtr->next != NULL) 
        {
            IQ_currPtr = IQ_currPtr->next;
        }
        
        // insert it 
        IQ_currPtr->next = iq;
    }
}

IQdata *issue_Instruction(int fu) 
{



    IQdata *result = NULL;
    if (is_IQ_Empty()) 
    {
        //printf("%s", "List is empty");
        return NULL;
    }
    
    iqPtr prevPtr = NULL;
    iqPtr IQ_currPtr = iqPtr_head;
    int i = 0;
    while(IQ_currPtr != NULL) 
    {
        
        /*To issue instruction for IU*/
        if(IQ_currPtr->allocatedBit == 1 && IQ_currPtr->src1Ready == 1 && IQ_currPtr-> src2Ready ==1 )
        {
            if (IQ_currPtr->fuType == fu)
            {
                result = IQ_currPtr;
            }
            else
            {
                return NULL;
            }
            // result = IQ_currPtr;
        }

        
        // /*To issue instruction for MU*/
        // else if(IQ_currPtr->allocatedBit == 1 && IQ_currPtr->src1Ready == 1 && IQ_currPtr-> src2Ready==1 && IQ_currPtr->fuType ==1)
        // {
        //     result = IQ_currPtr;
        // } 
        // // To issue instruction to  BU
        // else if(IQ_currPtr->allocatedBit == 1 && IQ_currPtr->src1Ready == 1 && IQ_currPtr-> src2Ready ==1 && IQ_currPtr->fuType ==2)
        // {
        //     result = IQ_currPtr;
        // }
        
        if(i != 0 && IQ_currPtr->allocatedBit == 1)
        {
            prevPtr = IQ_currPtr;
        }
        i++;
        IQ_currPtr = IQ_currPtr->next;       
    }
    
    if (result == NULL) 
    {
        //printf("%s", "Instruction is not found in the list");
        return NULL;
    }
    
    if (prevPtr == NULL) 
    {
        // this is the first instruction
        iqPtr_head = iqPtr_head->next;
        free(IQ_currPtr);
        IQ_currPtr = NULL;
        //return;
    }
    else if (result->next == NULL) 
    {
        // this is the last instruction
        prevPtr->next = NULL;
        free(IQ_currPtr);
        IQ_currPtr = NULL;
    } 
    else 
    {
        // anywhere in between first and last
        iqPtr nextPtr = result->next;
        prevPtr->next = nextPtr;
        free(IQ_currPtr);
        IQ_currPtr = NULL;
    }
    //printf("Instruction issued for execution\n" );
    curr_iqSize--;
    return result;
}

 void flush_IQ(){
    iqPtr IQ_currPtr = iqPtr_head;
    iqPtr nextPtr = IQ_currPtr->next;
    IQ_currPtr->next = nextPtr;
    free(IQ_currPtr);
    IQ_currPtr = NULL;
}