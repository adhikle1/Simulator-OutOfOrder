/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int prs1;   //physical register for source 1
    int prs2;   //physical register for source 2
    int prs1_valid;   // Valid physical register for source 1
    int prs2_valid;   // Valid physical register for source 2
    int rd;
    int prd;    //physical register for destination
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int fuType;        /*FU needed for instruction - 0 -> IU, 1 -> MU, 2 -> BU*/
    int lsqIndex;     /*LSQ index, -1 for non L/S instructions*/
    int memory_address;
    int has_insn;
    int lsBit;  //to indicate load store instruction
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int pregs[PREG_FILE_SIZE];       /* Integer register file */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int data_memory_valid_bit[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int positive_flag;
    int fetch_from_next_cycle;
    int renameTable[REG_FILE_SIZE];    /*Array for Rename Table*/
    int pr_valid_bit[PREG_FILE_SIZE];    /*Array for Physical Register Valid bit 0 - Invalid 1 - Valid*/
    int is_IU_free;             /*Flag to check if IU is free 0 - Busy 1 - free*/
    int is_MU_free;             /*Flag to check if MU is free 0 - Busy 1 - free*/
    int is_BU_free;             //Flag to check if BU is free 0 - Busy 1 -  
    int is_MEM_free;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage DR1;
    CPU_Stage RD2;
    CPU_Stage issue;
    CPU_Stage IU;
    CPU_Stage IUforward;
    CPU_Stage MU;
    CPU_Stage MUforward;
    CPU_Stage BU;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage memoryForward;
    CPU_Stage ROBcommit;
    CPU_Stage writeback;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
#endif
