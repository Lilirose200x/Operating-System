#include "pcb.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "shellmemory.h"

void destroyPageTable(PAGE_TABLE *page_tbl) {
	PAGE_TABLE *ptr;

	if (!page_tbl)
		return;

	ptr = page_tbl;
	page_tbl = page_tbl->next;
	ptr->frame_number = -1;
	ptr->last_used = 0;
	ptr->next = NULL;

	while (page_tbl) {
		ptr = page_tbl;
		page_tbl = page_tbl->next;
		free(ptr);
	}
}

//In this implementation, Pid is the same as file ID 
PCB* makePCB(int start, int end, char* pid, FILE *page_file){
    PCB * newPCB = malloc(sizeof(PCB));
    newPCB->pid = pid;
    newPCB->vm_PC = start;
    newPCB->vm_start  = start;
    newPCB->vm_end = end;
    newPCB->job_length_score = 1+end-start;
	newPCB->page_file = page_file;
	newPCB->page_tbl.frame_number = -1;
	newPCB->page_tbl.last_used = 0;
	newPCB->page_tbl.next = NULL;
    return newPCB;
}

unsigned int get_current_time(){
	//
	static unsigned int s_next_time = 0;
	return (++s_next_time);
}

int virtual_to_physical_addr(PCB *pPCB, int vm_PC, int *pbSwapped){
	int nStart, nEnd, rc;
	int phy_PC = -1;
	char *varName;
	int framePcbIndex = (vm_PC - pPCB->vm_start) / FRAME_LINE_COUNT;
	int frameOffset = (vm_PC - pPCB->vm_start) % FRAME_LINE_COUNT;
	PAGE_TABLE *prev = NULL;
	PAGE_TABLE *cur = &pPCB->page_tbl;
	for (; (cur); framePcbIndex--){
		if (0 == framePcbIndex)
			break;
		prev = cur;
		cur = cur->next;
	}
	if (cur) {
		if(cur->frame_number < 0){
			//Error, quit the processor
			raise_page_fault(pPCB, vm_PC);
			printf("Need load pagefile!\n");
			exit(-1);
		}
		else{
			phy_PC = ((cur->frame_number * FRAME_LINE_COUNT) + frameOffset);
			varName = mem_get_var_by_line(phy_PC);
			if (0 == strcmp(varName, pPCB->pid)){
				cur->last_used = get_current_time();
				mem_set_last_used_by_line(phy_PC, cur->last_used);
			}
			else {
				//Page fault
				raise_page_fault(pPCB, vm_PC);
				rc = add_file_to_mem(pPCB->page_file, &nStart, &nEnd, pPCB->pid, FRAME_LINE_COUNT, 1);
				
				if (rc) {
					//Memory overflow, quit the processor
					printf("Need switch pagefile!\n");
					exit(-1);
				}

				if (pbSwapped)
					(*pbSwapped) = 1;
				phy_PC = (nStart + frameOffset);
				cur->frame_number = (nStart / FRAME_LINE_COUNT);
				cur->last_used = get_current_time();
				mem_set_last_used_by_line(phy_PC, cur->last_used);
			}
		}
	}
	else {
		//Page fault
		raise_page_fault(pPCB, vm_PC);
		rc = add_file_to_mem(pPCB->page_file, &nStart, &nEnd, pPCB->pid, FRAME_LINE_COUNT, 1);
		
		if (rc) {
			//Memory overflow, quit the processor
			printf("Need switch pagefile!\n");
			exit(-1);
		}

		if (pbSwapped)
			(*pbSwapped) = 1;
		if (prev){
			cur = (PAGE_TABLE *)malloc(sizeof(PAGE_TABLE));
			cur->frame_number = (nStart / FRAME_LINE_COUNT);
			cur->last_used = get_current_time();
			cur->next = NULL;
			prev->next = cur;
		}
		else {
			cur = &pPCB->page_tbl;
			cur->frame_number = (nStart / FRAME_LINE_COUNT);
			cur->last_used = get_current_time();
			cur->next = NULL;
		}
		phy_PC = (nStart + frameOffset);
		mem_set_last_used_by_line(phy_PC, cur->last_used);
	}
	return phy_PC;
}
