/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
//Saurabh Naik , Aditya Dhikle, Nikhil Kulkarni
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

#include "free_list.c"
#include "issue_queue.c"
#include "rob.c"
#include "lsq.c"

int IU_counter; /*To count stages in IU*/
int MU_counter; /*To count stages in Mul*/
int MEM_counter; /*To count stages in Mul*/

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LOAD:
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_CMP:
        {
           printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BNP:
        case OPCODE_BP:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }
        
        case OPCODE_NOP:
        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }
        case OPCODE_RET:
        {
             printf("%s,R%d", stage->opcode_str, stage->rs1);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Architectural Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d]", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d]", i, cpu->regs[i]);
    }

    printf("----------\n%s\n----------\n", "Physical Registers:");

    for (int i = 0; i < PREG_FILE_SIZE / 2; ++i)
    {
        printf("P%-3d[%-3d], %d ", i, cpu->pregs[i], cpu->pr_valid_bit[i]);
    }

    printf("\n");

    for (i = (PREG_FILE_SIZE / 2); i < PREG_FILE_SIZE; ++i)
    {
        printf("P%-3d[%-3d], %d", i, cpu->pregs[i], cpu->pr_valid_bit[i]);
    }

    printf("\n\nIssue Queue: \n");
    
    /*Printing Issue Queue*/
    iqPtr currPtr = iqPtr_head;
    while (currPtr != NULL) 
    {
        printf("AL: %d PC: %d FU: %d Ins: %s Dest: P%d IMM: %d Src1: P%d V1: %d Src2: P%d V2: %d LS: %d\n", currPtr->allocatedBit,currPtr->pc, currPtr->fuType,currPtr->opcode_str, currPtr->dest,currPtr->imm, currPtr->prs1,currPtr->prs1Value, currPtr->prs2,currPtr->prs2Value, currPtr->lsqIndex);
        currPtr = currPtr->next;
    }

    printf("\n\nRename Table: \n");

    printf("\nFree List:\n");
    printf("[ ");
        for (i = FL_head; i != FL_tail; i = (i + 1) % PREG_FILE_SIZE)
            printf("P%d ", i);
        printf("P%d ]\n\n", i);

    
    /*Printing Rename Table*/
    printf("[ ");
    for(int i=0; i<REG_FILE_SIZE; i++)
        if (cpu->renameTable[i] != -1)
        {
            printf("R%d - P%d ", i,cpu->renameTable[i]);
        }
    printf("]\n");

    printf("\nROB:\n");
        for (i = rob_head; i != rob_tail; i = (i + 1) % rob_SIZE)
            printf("Allocated: %d Dest: R%d Physical: P%d Ins: %s PC:%d Result: %d LSQindex: %d\n", rob[i].allocated, rob[i].rd, rob[i].prd, rob[i].opcode_str, rob[i].pc_value, rob[i].result, rob[i].lsqindex);
        printf("Allocated: %d Dest: R%d Physical: P%d Ins: %s PC: %d Result: %d LSQindex: %d\n\n", rob[i].allocated, rob[i].rd, rob[i].prd, rob[i].opcode_str, rob[i].pc_value,rob[i].result, rob[i].lsqindex);

    printf("Current IQ size: %d \n", curr_iqSize);

     printf("\nLSQ: LSQ head: %d\n", lsq_head);
        for (i = lsq_head; i != lsq_tail; i = (i + 1) % lsq_SIZE)
            printf("Allocated: %d lsbit: %d Prd: P%d Prs1: P%d = %d V: %d Mem_Add: %d V: %d PC: %d\n", lsq[i].allocated,lsq[i].lsBit, lsq[i].prd, lsq[i].prs1,lsq[i].prs1_value,lsq[i].data_valid_Bit, lsq[i].mem_address, lsq[i].mem_address_vBit, lsq[i].pc_value);
        printf("Allocated: %d lsbit: %d Prd: P%d Prs1: P%d =%d V: %d Mem_Add: %d V: %d PC: %d\n", lsq[i].allocated,lsq[i].lsBit, lsq[i].prd, lsq[i].prs1, lsq[i].prs1_value,lsq[i].data_valid_Bit, lsq[i].mem_address, lsq[i].mem_address_vBit, lsq[i].pc_value);

}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;

        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        cpu->DR1 = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_DR1(APEX_CPU *cpu)

{
    if (cpu->DR1.has_insn)
    {
        if(!is_IQ_Full(curr_iqSize) && !checkEmpty_FL(FL_head) && !rob_checkFull())
        {
            /* Read operands from register file based on the instruction type */
            switch (cpu->DR1.opcode)
            {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_OR:
            case OPCODE_AND:
            case OPCODE_XOR:
            case OPCODE_MUL:
            {
                cpu->DR1.prd = get_free_physical_register();
                cpu->DR1.prs1 = cpu->renameTable[cpu->DR1.rs1];
                cpu->DR1.prs2 = cpu->renameTable[cpu->DR1.rs2];
                cpu->DR1.rs1_value = cpu->pregs[cpu->DR1.prs1];
                cpu->DR1.prs1_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs1]];
                cpu->DR1.rs2_value = cpu->pregs[cpu->DR1.prs2];
                cpu->DR1.prs2_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs2]];
                cpu->renameTable[cpu->DR1.rd] = cpu->DR1.prd;
                cpu->pr_valid_bit[cpu->DR1.prd] = 0;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LOAD:
            case OPCODE_JALR:
            {
                cpu->DR1.prd = get_free_physical_register();
                cpu->DR1.rs1_value = cpu->pregs[cpu->renameTable[cpu->DR1.rs1]];
                cpu->DR1.prs1_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs1]];
                cpu->DR1.prs2_valid = 1;
                cpu->renameTable[cpu->DR1.rd] = cpu->DR1.prd;
                cpu->pr_valid_bit[cpu->DR1.prd] = 0;
                break;
            }

            case OPCODE_CMP:
            {
                cpu->DR1.prs1 = cpu->renameTable[cpu->DR1.rs1];
                cpu->DR1.prs2 = cpu->renameTable[cpu->DR1.rs2];
                cpu->DR1.rs1_value = cpu->pregs[cpu->DR1.prs1];
                cpu->DR1.prs1_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs1]];
                cpu->DR1.rs2_value = cpu->pregs[cpu->DR1.prs2];
                cpu->DR1.prs2_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs2]];
                break;
            }
            case OPCODE_RET:
            {
                cpu->DR1.prs1 = cpu->renameTable[cpu->DR1.rs1];
                cpu->DR1.rs1_value = cpu->pregs[cpu->DR1.prs1];
                cpu->DR1.prs1_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs1]];
                cpu->DR1.prs2_valid = 1;
                break;

            }
            case OPCODE_STORE:
            {
                cpu->DR1.prs1 = cpu->renameTable[cpu->DR1.rs1];
                cpu->DR1.prs2 = cpu->renameTable[cpu->DR1.rs2];
                cpu->DR1.rs1_value = cpu->pregs[cpu->DR1.prs1];
                cpu->DR1.prs1_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs1]];
                cpu->DR1.rs2_value = cpu->pregs[cpu->DR1.prs2];
                cpu->DR1.prs2_valid = cpu->pr_valid_bit[cpu->renameTable[cpu->DR1.rs2]];
                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                cpu->DR1.prd = get_free_physical_register();
                cpu->DR1.prs1_valid = 1;
                cpu->DR1.prs2_valid = 1;
                cpu->renameTable[cpu->DR1.rd] = cpu->DR1.prd;
                cpu->pr_valid_bit[cpu->DR1.prd] = 0;
                break;
            }
            }

            /* Copy data from decode latch to execute latch*/
            cpu->RD2 = cpu->DR1;
            cpu->DR1.has_insn = FALSE;

            if (ENABLE_DEBUG_MESSAGES)
            {
                print_stage_content("DR1", &cpu->DR1);
            }

        }
        
    }
}

static void
APEX_RD2(APEX_CPU *cpu)
{
    if (cpu->RD2.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->RD2.opcode)
        {
            //FU type 0 - IU
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_MOVC:
            case OPCODE_SUB:
            case OPCODE_SUBL:
            case OPCODE_XOR:
            case OPCODE_OR:
            case OPCODE_AND:
            case OPCODE_CMP:
            case OPCODE_NOP:
            case OPCODE_HALT:
            {
                cpu->RD2.fuType =0;
                cpu->RD2.lsqIndex = -1;
                iqEntry(cpu->RD2);
                rob_Entry(cpu->RD2);
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->RD2.fuType =0;
                cpu->RD2.lsBit =0;
                lsq_Entry(cpu->RD2);
                cpu->RD2.lsqIndex = lsq_tail;
                iqEntry(cpu->RD2);
                rob_Entry(cpu->RD2);
                break;
            }

            case OPCODE_STORE:
            {
                cpu->RD2.fuType =0;
                cpu->RD2.lsBit =1;
                lsq_Entry(cpu->RD2);
                cpu->RD2.lsqIndex = lsq_tail;
                iqEntry(cpu->RD2);
                rob_Entry(cpu->RD2);
                break;
            }
            
            //FU type 1 - MU
            case OPCODE_MUL:
            {
                cpu->RD2.fuType =1;
                cpu->RD2.lsqIndex = -1;
                iqEntry(cpu->RD2);
                rob_Entry(cpu->RD2);
                break;
            }
            

            //FU type 2 - BU
            
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_JUMP:
            case OPCODE_JALR:
            case OPCODE_RET:
            {
                cpu->RD2.fuType =2;
                cpu->RD2.lsqIndex = -1;
                iqEntry(cpu->RD2);
                rob_Entry(cpu->RD2);
                break;
            }

            
        }

        /* Copy data from decode latch to execute latch*/
        cpu->issue = cpu->RD2;
        cpu->RD2.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("RD2", &cpu->RD2);
        }
    }
}

static void
APEX_issue(APEX_CPU *cpu)
{
    if (cpu->issue.has_insn)
    {
        
        /*Allocating the instruction in ROB*/
        iqPtr currPtr = iqPtr_head;
        while (currPtr != NULL) 
        {
            if(!currPtr->allocatedBit)
            {
                currPtr->allocatedBit = 1;
                break;
            }
            currPtr = currPtr->next;
        }

        /* Issuing instruction for IU*/
        currPtr = iqPtr_head;
        if(cpu->is_IU_free)
        {
            currPtr = issue_Instruction(0);
            if(currPtr != NULL && currPtr->fuType==0)
            {
              cpu->issue.pc = currPtr->pc;
              strcpy(currPtr->opcode_str, cpu->issue.opcode_str);
              cpu->issue.opcode = currPtr->opcode;
              cpu->issue.fuType = currPtr->fuType;
              cpu->issue.imm = currPtr->imm; /*Literal value*/
              cpu->issue.prs1 = currPtr->prs1;   /*Source 1 Tag*/
              cpu->issue.rs1_value = currPtr->prs1Value;  /*Source 1 Value*/
              cpu->issue.prs1 = currPtr->prs1;   /*Source 2 Tag*/
              cpu->issue.rs2_value = currPtr->prs2Value;  /*Source 2 Value*/
              cpu->issue.prd = currPtr->dest; /*Destination Physical Register*/
              cpu->IU = cpu->issue;
              cpu->IU.has_insn =1;
            }

        }    
        /* Issuing instruction for MU*/
        currPtr = iqPtr_head;
        if(cpu->is_MU_free)
        {
            currPtr = issue_Instruction(1);
            if(currPtr != NULL && currPtr->fuType == 1)
            {
              //cpu->issue.allocatedBit = 0;
              cpu->issue.pc = currPtr->pc;
              strcpy(currPtr->opcode_str, cpu->issue.opcode_str);
              cpu->issue.opcode = currPtr->opcode;
              cpu->issue.fuType = currPtr->fuType;
              cpu->issue.imm = currPtr->imm; /*Literal value*/
              cpu->issue.prs1 = currPtr->prs1;   /*Source 1 Tag*/
              cpu->issue.rs1_value = currPtr->prs1Value;  /*Source 1 Value*/
              cpu->issue.prs1 = currPtr->prs1;   /*Source 2 Tag*/
              cpu->issue.rs2_value = currPtr->prs2Value;  /*Source 2 Value*/
              cpu->issue.prd = currPtr->dest; /*Destination Physical Register*/
              cpu->MU = cpu->issue;
              cpu->MU.has_insn =TRUE;
              cpu->is_MU_free = 0;
              MU_counter=1;
            }
        }

        /* Issuing instruction for BU*/
        //printf("BU Free %d", cpu->is_BU_free);
        currPtr = iqPtr_head;
        if(cpu->is_BU_free)
        {
            currPtr = issue_Instruction(2);
            if(currPtr != NULL && currPtr->fuType == 2)
            {
              printf("IN BU IF\n");   
              //cpu->issue.allocatedBit = 0;
              cpu->issue.pc = currPtr->pc;
              strcpy(currPtr->opcode_str, cpu->issue.opcode_str);
              cpu->issue.opcode = currPtr->opcode;
              cpu->issue.fuType = currPtr->fuType;
              cpu->issue.imm = currPtr->imm; /*Literal value*/
              cpu->issue.prs1 = currPtr->prs1;   /*Source 1 Tag*/
              cpu->issue.rs1_value = currPtr->prs1Value;  /*Source 1 Value*/
              cpu->issue.prs1 = currPtr->prs1;   /*Source 2 Tag*/
              cpu->issue.rs2_value = currPtr->prs2Value;  /*Source 2 Value*/
              cpu->issue.prd = currPtr->dest; /*Destination Physical Register*/
              cpu->BU = cpu->issue;
              cpu->BU.has_insn =TRUE;
              //cpu->is_BU_free = 0;
              
            }
        }

        /* Issuing instruction for memory operation*/
        if(cpu->is_MEM_free)
        {
            /*LOAD instruction*/
            if(lsq[lsq_head].allocated == 1 && lsq[lsq_head].lsBit ==0)
            {
                int i;
                for (i = rob_head; i != rob_tail; i = (i + 1) % rob_SIZE)
                {
                    printf("ROB pc: %d, lsq head: %d lsqPC:%d\n", rob[i].pc_value, lsq_head, lsq[lsq_head].pc_value);
                    if(rob[i].pc_value == lsq[lsq_head].pc_value)
                    {
                        /* Copy data from ROB latch to memory latch for LOAD */
                        cpu->issue.memory_address=lsq[lsq_head].mem_address;
                        cpu->issue.pc = rob[i].pc_value;
                        strcpy(cpu->issue.opcode_str, rob[i].opcode_str);
                        cpu->issue.opcode = rob[i].opcode;
                        cpu->issue.rs1_value = lsq[lsq_head].prs1_value;
                        lsq_to_mem();
                        
                        MEM_counter =1;
                        cpu->is_MEM_free=0;
                        cpu->memory = cpu->issue;
                        break;
                    }
                }
                /*Checking if tail of ROB is ready for memory operation*/
                if(rob[i].pc_value == lsq[lsq_tail].pc_value)
                {
                    /* Copy data from ROB latch to memory latch for LOAD */
                    cpu->issue.memory_address=lsq[lsq_head].mem_address;
                    cpu->issue.pc = rob[i].pc_value;
                    strcpy(cpu->issue.opcode_str, rob[i].opcode_str);
                    cpu->issue.opcode = rob[i].opcode;
                    cpu->issue.rs1_value = lsq[lsq_head].prs1_value;
                    
                    lsq_to_mem();
                    MEM_counter =1;
                    cpu->is_MEM_free=0;
                    cpu->memory = cpu->issue;
                    
                }
            }
            /*Store*/
            else if(lsq[lsq_head].allocated ==1 && lsq[lsq_head].data_valid_Bit==1 && lsq[lsq_head].lsBit ==1 && rob[rob_head].lsqindex == lsq_head)
            {
                /* Copy data from ROB latch to memory latch for Store */
                cpu->issue.memory_address=lsq[lsq_head].mem_address;
                cpu->issue.pc = rob[rob_head].pc_value;
                strcpy(cpu->issue.opcode_str, rob[rob_head].opcode_str);
                cpu->issue.opcode = rob[rob_head].opcode;
                cpu->issue.rs1_value = lsq[lsq_head].prs1_value;
                
                lsq_to_mem();
                MEM_counter =1;
                cpu->is_MEM_free=0;
                cpu->memory = cpu->issue;
                cpu->memory.has_insn = TRUE;
            }
        }

        if(lsq[lsq_head].mem_address_vBit !=0)
        {
            lsq[lsq_head].allocated=1;
        }
        
        //cpu->issue.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("issue", &cpu->issue);
        }
    }
}

static void
APEX_IU(APEX_CPU *cpu)
{
    if (cpu->IU.has_insn)
    {
        /* IU logic based on instruction type */
        switch (cpu->IU.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value + cpu->IU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                

                break;
            }

            case OPCODE_ADDL:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value + cpu->IU.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->IU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUB:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value - cpu->IU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->IU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUBL:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value - cpu->IU.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->IU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_CMP:
            {
                if (cpu->IU.rs1_value == cpu->IU.rs2_value)
                {
                    cpu->zero_flag = TRUE;
                }
                else if(cpu->IU.rs1_value > cpu->IU.rs2_value)
                {
                    cpu->positive_flag = TRUE;
                }
                break;
            }

            case OPCODE_AND:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value & cpu->IU.rs2_value;
                break;
            }
            case OPCODE_OR:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value | cpu->IU.rs2_value;
                break;
            }

            case OPCODE_XOR:
            {
                cpu->IU.result_buffer
                    = cpu->IU.rs1_value ^ cpu->IU.rs2_value;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->IU.memory_address
                    = cpu->IU.rs1_value + cpu->IU.imm;
                break;
            }

            case OPCODE_STORE:
            {
                cpu->IU.memory_address
                    = cpu->IU.rs2_value + cpu->IU.imm;
                break;
            }
            case OPCODE_MOVC: 
            {
                cpu->IU.result_buffer = cpu->IU.imm;
                //printf("\nResult: %d\n", cpu->IU.result_buffer);

                /* Set the zero flag based on the result buffer */
                if (cpu->IU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
        }

        /* Copy data from IU latch to IU Forward latch*/
        cpu->IUforward = cpu->IU;
        cpu->IU.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("IU", &cpu->IU);
        }
    }
}

static void
APEX_MU(APEX_CPU *cpu)
{
    if (cpu->MU.has_insn)
    {
        if(MU_counter%4 == 0)
        {
           /* IU logic based on instruction type */
           switch (cpu->MU.opcode)
          {
            case OPCODE_MUL:
            {
                cpu->MU.result_buffer
                    = cpu->MU.rs1_value * cpu->MU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->MU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
          }

          /* Copy data from MU latch to MU Forward latch*/
          cpu->MUforward = cpu->MU;
          cpu->is_MU_free=1;
          cpu->MU.has_insn = FALSE;
        }
        printf("\nMU Stage: %d\n",MU_counter);
        MU_counter++;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MU", &cpu->MU);
        }
    }
}


static void
APEX_BU(APEX_CPU *cpu)
{
    if (cpu->BU.has_insn)
    {
        
           /* IU logic based on instruction type */
           switch (cpu->BU.opcode)
          {
            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->BU.pc + cpu->BU.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->DR1.has_insn = FALSE;
                    cpu->RD2.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                    

                    while(iqPtr_head != NULL){
                        flush_IQ();
                    }
                }
                break;
            }
            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->BU.pc + cpu->BU.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->DR1.has_insn = FALSE;
                    cpu->RD2.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BP:
            {
                if (cpu->positive_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->BU.pc + cpu->BU.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->DR1.has_insn = FALSE;
                    cpu->RD2.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
	     case OPCODE_BNP:
            {
                if (cpu->positive_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->BU.pc + cpu->BU.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->DR1.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
          }

          
          
        }
        rob_update(cpu->BU.pc, cpu->BU.result_buffer);
        cpu->is_BU_free=1;
        cpu->BU.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("BU", &cpu->BU);
        }
}

static int
APEX_IUforward(APEX_CPU *cpu)
{
    if (cpu->IUforward.has_insn)
    {
        //To status for Non LSQ entry
        if(cpu->IUforward.lsqIndex == -1)
        {
            cpu->pregs[cpu->IUforward.prd] = cpu->IUforward.result_buffer;
            rob_update(cpu->IUforward.pc, cpu->IUforward.result_buffer);
            cpu->pr_valid_bit[cpu->IUforward.prd] = 1;
            put_physicalRegister_inFL(cpu->IUforward.prd);
            IQupdate(cpu->IUforward.prd, cpu->IUforward.result_buffer);  
            lsq_update(cpu->IUforward.prd, cpu->IUforward.result_buffer);          
        }
        /*for Load store update LSQ*/
        else
        {
            //lsq[cpu->IUforward.lsqIndex].allocated = 1;
            lsq[cpu->IUforward.lsqIndex].mem_address = cpu->IUforward.memory_address;
            lsq[cpu->IUforward.lsqIndex].mem_address_vBit = 1;
            switch (cpu->IUforward.opcode)
            {
            case OPCODE_LOAD:
                put_physicalRegister_inFL(cpu->IUforward.prd);
                break;
            }
        }
        cpu->ROBcommit.has_insn = TRUE;
        cpu->IUforward.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("IU Forward", &cpu->IUforward);
        }

    }
}

static int
APEX_MUforward(APEX_CPU *cpu)
{
    if (cpu->MUforward.has_insn)
    {
        cpu->pregs[cpu->MUforward.prd] = cpu->MUforward.result_buffer;
        rob_update(cpu->MUforward.pc, cpu->MUforward.result_buffer);
        cpu->pr_valid_bit[cpu->MUforward.prd] = 1;
        IQupdate(cpu->MUforward.prd, cpu->MUforward.result_buffer);

        cpu->MUforward.has_insn = FALSE;
        cpu->ROBcommit.has_insn = TRUE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("MU Forward", &cpu->MUforward);
        }

    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        if(MEM_counter%2 == 0)
        {
            switch (cpu->memory.opcode)
         {            
            case OPCODE_LOAD:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                break;
            }
            case OPCODE_STORE:
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                printf("\nMemory location: D[%d]: %d\n",cpu->memory.memory_address,cpu->memory.rs1_value);
                break;
            }
          }
          
          cpu->is_MEM_free=1;
          cpu->memoryForward = cpu->memory;
          cpu->memoryForward.has_insn = TRUE;
          cpu->memory.has_insn = FALSE;
        }
        printf("MEM Stage: %d\n", MEM_counter);
        MEM_counter++;
        
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
}

static void
APEX_memoryForward(APEX_CPU *cpu)
{
    if (cpu->memoryForward.has_insn)
    {   
        switch (cpu->memoryForward.opcode)
        {            
            case OPCODE_LOAD:
            {
                /* Read from data memoryForward */
                int i;
                for (i = rob_head; i != rob_tail; i = (i + 1) % rob_SIZE)
                {
                    if(rob[i].pc_value == cpu->memoryForward.pc)
                    {
                        rob[i].allocated = 1;
                        rob[i].result = cpu->memoryForward.result_buffer;
                        break;
                    }
                }
                /*for tail entry of ROB*/
                if(rob[i].pc_value == cpu->memoryForward.pc)
                    {
                        rob[i].allocated = 1;
                        rob[i].result = cpu->memoryForward.result_buffer;
                        break;
                    }
                
                break;
            }
            case OPCODE_STORE:
            {
                rob[rob_head].allocated = 1;
            }
        }

        /*int i;
        for (i = rob_head; i != rob_tail; i = (i + 1) % rob_SIZE)
        {
            if(rob[i].lsqindex == cpu->IUforward.lsqIndex)
            {
                rob[i].allocated =1;
                break;
            }
        }
        if(rob[i].lsqindex == cpu->IUforward.lsqIndex)
        {
            rob[i].allocated = 1;
        }
        */
        

        cpu->memoryForward.has_insn = FALSE;
        cpu->ROBcommit.has_insn = TRUE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("memoryForward", &cpu->memoryForward);
        }
    }
}

/*ROB Commit*/
static void
APEX_ROBcommit(APEX_CPU *cpu)
{
    if (cpu->ROBcommit.has_insn)
    {
        if(rob[rob_head].allocated == 1)
        {
            cpu->ROBcommit.result_buffer =rob[rob_head].result;
            cpu->ROBcommit.rd =rob[rob_head].rd;
            cpu->ROBcommit.opcode =rob[rob_head].opcode;
            cpu->ROBcommit.pc =rob[rob_head].pc_value;
            strcpy(cpu->ROBcommit.opcode_str, rob[rob_head].opcode_str);
            rob_Commitment();
            /* Copy data from ROB latch to writeback latch*/
            cpu->writeback = cpu->ROBcommit;
            // cpu->ROBcommit.has_insn = FALSE;
            if (rob_checkEmpty())
            {
                /* code */
                cpu->ROBcommit.has_insn = FALSE;
            }
            
        }
        /*Load*/
    }
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("ROB Commit", &cpu->ROBcommit);
        }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_MUL:
            case OPCODE_MOVC:
            case OPCODE_XOR:
            case OPCODE_OR:
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    cpu->is_IU_free = 1;
    cpu->is_MU_free = 1;
    cpu->is_MEM_free = 1;
    cpu->is_BU_free = 1;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }
    
    /*Initialiazing Rename Table*/
    for (int i = 0; i < REG_FILE_SIZE; i++)
    {
        cpu->renameTable[i] = -1;
    }
    /*Initialiazing Physical register valid bit*/
    for (int i = 0; i < PREG_FILE_SIZE; i++)
    {
        cpu->pr_valid_bit[i] = 1;
    }

    /*Initialiazing Data Memory valid bit*/
    for (int i = 0; i < DATA_MEMORY_SIZE; i++)
    {
        cpu->data_memory_valid_bit[i] = 1;
    }

    /*Intializing free list*/
    FL_head = FL_tail =-1;
    for (int i = 0; i <= PREG_FILE_SIZE; i++)
    {
        put_physicalRegister_inFL(i);
    }
    
    /*Initializing Issue Queue*/
    iqPtr_head = NULL;
    curr_iqSize = 0;

    /*Initializing ROB*/
    rob_head = rob_tail =-1;
    rob = calloc(rob_SIZE, sizeof(ROBData));

    /*Initializing LSQ*/
    lsq_head = lsq_tail =-1;
    lsq = calloc(lsq_SIZE, sizeof(LSQData));

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        
        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        //APEX_execute(cpu);
        APEX_ROBcommit(cpu);
        APEX_memoryForward(cpu);
        APEX_MUforward(cpu);
        APEX_MU(cpu);
        APEX_BU(cpu);
        APEX_IUforward(cpu); 
        APEX_IU(cpu);
        APEX_issue(cpu);
        APEX_memory(cpu);
        APEX_RD2(cpu);
        APEX_DR1(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}