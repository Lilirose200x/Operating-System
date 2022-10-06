#ifndef PCB_H
#define PCB_H
#include <stdio.h>

typedef struct tagPAGE_TABLE {
	unsigned int frame_number;
	unsigned int last_used;
	struct tagPAGE_TABLE *next;
} PAGE_TABLE;

void destroyPageTable(PAGE_TABLE *page_tbl);

/*
 * Struct:  PCB 
 * --------------------
 * pid: process(task) id
 * PC: program counter, stores the line that the task is executing
 * start: the first line in shell memory that belongs to this task
 * end: the last line in shell memory that belongs to this task
 * job_length_score: for EXEC AGING use only, stores the job length score
 */
typedef struct
{
    char* pid;  
    int vm_PC;     
    int vm_start;  
    int vm_end;    
    int job_length_score;  
	FILE *page_file;       
	PAGE_TABLE page_tbl; 
}PCB;

PCB * makePCB(int start, int end, char* pid, FILE *page_file);
int virtual_to_physical_addr(PCB *pPCB, int vm_PC, int *pbSwapped);

#endif
