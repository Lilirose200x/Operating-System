#include "pcb.h"
#include "kernel.h"
#include "cpu.h"
#include "shell.h"
#include "shellmemory.h"
#include "interpreter.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define QUEUE_LENGTH 10
#define MAX_INT 2147483646
PCB* readyQueue[QUEUE_LENGTH];

void ready_queue_initialize()
{
	size_t i;
    for (i = 0; i < QUEUE_LENGTH; ++i)
    {
        readyQueue[i] = (PCB*)malloc(sizeof(PCB));
        (*readyQueue[i]).vm_PC = -1;
        (*readyQueue[i]).vm_start = -1;
        (*readyQueue[i]).vm_end = -1;
        (*readyQueue[i]).pid = NULL;
        (*readyQueue[i]).job_length_score = -1;
		readyQueue[i]->page_file = NULL;
		readyQueue[i]->page_tbl.frame_number = -1;
		readyQueue[i]->page_tbl.last_used = 0;
		readyQueue[i]->page_tbl.next = NULL;
    }
}

void ready_queue_Empty(){
	size_t i;
    for (i = 0; i < QUEUE_LENGTH; ++i)
    {
        (*readyQueue[i]).vm_PC = -1;
        (*readyQueue[i]).vm_start = -1;
        (*readyQueue[i]).vm_end = -1;
        (*readyQueue[i]).pid = NULL;
        (*readyQueue[i]).job_length_score = -1;
		if (readyQueue[i]->page_file) {
			fclose(readyQueue[i]->page_file);
			readyQueue[i]->page_file = NULL;
		}
		destroyPageTable(&readyQueue[i]->page_tbl);
    }
}

void ready_queue_destory()
{
	size_t i;
    for (i = 0; i < QUEUE_LENGTH; ++i)
    {
		if (readyQueue[i]->page_file) {
			fclose(readyQueue[i]->page_file);
			readyQueue[i]->page_file = NULL;
		}
		destroyPageTable(&readyQueue[i]->page_tbl);
        free(readyQueue[i]);
		readyQueue[i] = NULL;
    }
}

PCB ready_queue_pop(int index, bool inPlace)
{
	size_t i;
    PCB head = (*readyQueue[index]);
    if(inPlace){
        for (i = index+1; i < QUEUE_LENGTH; i++){
            (*readyQueue[i-1]).vm_PC = (*readyQueue[i]).vm_PC;
            (*readyQueue[i-1]).vm_start = (*readyQueue[i]).vm_start;
            (*readyQueue[i-1]).vm_end = (*readyQueue[i]).vm_end;
            (*readyQueue[i-1]).pid = (*readyQueue[i]).pid;
            (*readyQueue[i-1]).job_length_score = (*readyQueue[i]).job_length_score;
			readyQueue[i - 1]->page_file = readyQueue[i]->page_file;
			readyQueue[i - 1]->page_tbl = readyQueue[i]->page_tbl;
        }
        (*readyQueue[QUEUE_LENGTH-1]).vm_PC = -1;
        (*readyQueue[QUEUE_LENGTH-1]).vm_start = -1;
        (*readyQueue[QUEUE_LENGTH-1]).vm_end = -1;
        (*readyQueue[QUEUE_LENGTH-1]).pid = NULL;
        (*readyQueue[QUEUE_LENGTH-1]).job_length_score = -1;
		readyQueue[QUEUE_LENGTH - 1]->page_file = NULL;
		readyQueue[QUEUE_LENGTH - 1]->page_tbl.frame_number = -1;
		readyQueue[QUEUE_LENGTH - 1]->page_tbl.last_used = 0;
		readyQueue[QUEUE_LENGTH - 1]->page_tbl.next = NULL;
    }
    return head;
}

void ready_queue_add_to_end(PCB *pPCB)
{
	int i;
    for(i = 0; i < QUEUE_LENGTH; i++){
        if ( (*readyQueue[i]).vm_start == -1 ){
            (*readyQueue[i]).vm_PC = (*pPCB).vm_PC;
            (*readyQueue[i]).vm_start = (*pPCB).vm_start;
            (*readyQueue[i]).vm_end = (*pPCB).vm_end;
            (*readyQueue[i]).pid = (*pPCB).pid;
            (*readyQueue[i]).job_length_score = (*pPCB).job_length_score;
			readyQueue[i]->page_file = pPCB->page_file;
			readyQueue[i]->page_tbl = pPCB->page_tbl;
            break;
        }
    }
}

void ready_queue_add_to_front(PCB *pPCB){
	size_t i;
    for (i = QUEUE_LENGTH-1; i > 0; i--){
        (*readyQueue[i]).vm_PC = (*readyQueue[i-1]).vm_PC;
        (*readyQueue[i]).vm_start = (*readyQueue[i-1]).vm_start;
        (*readyQueue[i]).vm_end = (*readyQueue[i-1]).vm_end;
        (*readyQueue[i]).pid = (*readyQueue[i-1]).pid;
        (*readyQueue[i]).job_length_score = (*readyQueue[i-1]).job_length_score;
		readyQueue[i]->page_file = readyQueue[i - 1]->page_file;
		readyQueue[i]->page_tbl = readyQueue[i - 1]->page_tbl;
    }
    // readyQueue[0] = pPCB;
    (*readyQueue[0]).vm_PC = (*pPCB).vm_PC;
    (*readyQueue[0]).vm_start = (*pPCB).vm_start;
    (*readyQueue[0]).vm_end = (*pPCB).vm_end;
    (*readyQueue[0]).pid = (*pPCB).pid;
    (*readyQueue[0]).job_length_score = (*pPCB).job_length_score;
	readyQueue[0]->page_file = pPCB->page_file;
	readyQueue[0]->page_tbl = pPCB->page_tbl;
}

bool is_ready_empty(){
	size_t i;
    for (i = 0; i < QUEUE_LENGTH; ++i)
    {
        if((*readyQueue[i]).vm_start != -1){
            return false;
        }  
    }
    return true;
}

void terminate_task_in_queue_by_index(int i){
    (*readyQueue[i]).vm_start = -1;
    (*readyQueue[i]).vm_end = -1;
    (*readyQueue[i]).vm_PC = -1;
    (*readyQueue[i]).pid = NULL;
    (*readyQueue[i]).job_length_score = -1;
	if (readyQueue[i]->page_file) {
		fclose(readyQueue[i]->page_file);
		readyQueue[i]->page_file = NULL;
	}
	destroyPageTable(&readyQueue[i]->page_tbl);
}

int myinit(const char *filename) {
	FILE* fp;
	char* fileID;
	int error_code = 0;
	int* start = (int*)malloc(sizeof(int));
	int* end = (int*)malloc(sizeof(int));
	int num_lines = 0;
	int load_lines = 0;
	PCB* newPCB;

	//create pagefile on backing store
	fp = copy_file_to_backing_store(filename, &num_lines);
	if (fp == NULL) {
		error_code = 11; // 11 is the error code for file does not exist
		return error_code;
	}

	//generate a random ID as file ID
	fileID = (char*)malloc(32);
	sprintf(fileID, "%d", rand());

	error_code = add_file_to_mem(fp, start, end, fileID, (FRAME_LINE_COUNT * FRAME_INIT_COUNT), 0);
	if (error_code != 0) {
		fclose(fp);
		return error_code;
	}

	load_lines = (1 + *end - *start);
	if (load_lines < num_lines) {
		*end = (*start + num_lines - 1);
	}

	newPCB = makePCB(*start, *end, fileID, fp);
	newPCB->job_length_score = num_lines;

	if (newPCB->job_length_score) {
		int pos = *start;
		PAGE_TABLE *prev = NULL;
		PAGE_TABLE *cur = &newPCB->page_tbl;
		unsigned idx = 0;
		for (; idx < FRAME_INIT_COUNT; ++idx) {
			if (*end < pos)
				break;
			if (!cur) {
				cur = (PAGE_TABLE *)malloc(sizeof(PAGE_TABLE));
				prev->next = cur;
			}

			cur->frame_number = (pos / FRAME_LINE_COUNT);
			cur->last_used = 0;
			cur->next = NULL;

			prev = cur;
			cur = cur->next;
			pos += FRAME_LINE_COUNT;
		}
	}
    ready_queue_add_to_end(newPCB);
	free(newPCB);
	newPCB = NULL;
    return error_code;
}

int get_scheduling_policy_number(char* policy){
    if(strcmp("FCFS",policy)==0){
        return 0;
    }else if(strcmp("SJF",policy)==0){
        return 1;
    }else if(strcmp("RR",policy)==0){
        return 2;
    }else if(strcmp("AGING",policy)==0){
        return 3;
    }else{
        //error code 15
        return 15;
    }
}


/*
 * Variable:  schedulingPolicy 
 * --------------------
 * 0: FCFS
 * 1: SJF
 * 2: RR
 * 3: AGING
 */
int scheduler(int policyNumber){
    int error_code = 0;

    int cpu_quanta_per_program = 2;

    //FCFS and SJF: running program will stop when it finishes
    if( policyNumber == 0 || policyNumber == 1 ){
        cpu_quanta_per_program = MAX_INT;
    }else if(policyNumber == 3){
        cpu_quanta_per_program = 1;
    }

    //scheduling logic for 0: FCFS and 2: RR
    if(policyNumber == 0 || policyNumber == 2){
        //keep running programs while ready queue is not empty
        while(ready_queue_pop(0,false).vm_PC != -1)
        {
			int error_code_load_PCB_TO_CPU;
            PCB firstPCB = ready_queue_pop(0,false);
            load_PCB_TO_CPU(firstPCB.vm_PC);
            
            error_code_load_PCB_TO_CPU = cpu_run(&firstPCB, cpu_quanta_per_program, firstPCB.vm_end);
            
            if(error_code_load_PCB_TO_CPU == 2){
                //the head PCB program has been done, time to reclaim the shell mem
                clean_mem(&(firstPCB.page_tbl));
                PCB a = ready_queue_pop(0,true);
				if (a.page_file) {
					fclose(a.page_file);
					a.page_file = NULL;
				}
            }
            if(error_code_load_PCB_TO_CPU == 0){
                //the head PCB program has finished its quanta, it need to be put to the end of ready queue
                firstPCB.vm_PC = cpu_get_ip();
                ready_queue_pop(0,true);
                ready_queue_add_to_end(&firstPCB);
            }
        }
    }

    //scheduling policy for 1: SJF
    if(policyNumber == 1){
        while(!is_ready_empty())
        {
            //task with the lowest lines of codes runs first
            int task_index_with_the_least_lines;
            int task_lines = MAX_INT;
			int i;
			PCB current_task_PCB;
			int error_code_load_PCB_TO_CPU;

            //get the lowest job length 
            for(i = 0; i < QUEUE_LENGTH; i++){
                if((*readyQueue[i]).vm_start != -1 && 1 + (*readyQueue[i]).vm_end - (*readyQueue[i]).vm_start < task_lines){
                    task_lines = 1 + (*readyQueue[i]).vm_end - (*readyQueue[i]).vm_start;
                    task_index_with_the_least_lines = i;
                }
            }

            current_task_PCB = (*readyQueue[task_index_with_the_least_lines]);
            load_PCB_TO_CPU(current_task_PCB.vm_PC);
            
            error_code_load_PCB_TO_CPU = cpu_run(&current_task_PCB, cpu_quanta_per_program, current_task_PCB.vm_end);
            
            //the head PCB program has been done, time to reclaim the shell mem
            clean_mem(&current_task_PCB.page_tbl);
            //put the current PCB into invalid mode
            terminate_task_in_queue_by_index(task_index_with_the_least_lines);
        }
    }

    //scheduling policy for 3: Aging
    if(policyNumber == 3){
        int task_index_least_job_length_score;
        int task_job_length_score = MAX_INT;
		int i;
		PCB job_with_lowest_job_score;

        //find job with the lowest job score
        for(i = 0; i < QUEUE_LENGTH; i++){
            //get the lowest job length score
            if((*readyQueue[i]).vm_start != -1 && (*readyQueue[i]).job_length_score < task_job_length_score){
                task_job_length_score = (*readyQueue[i]).job_length_score;
                task_index_least_job_length_score = i;
            }
        }
        //move the task with the lowest job score to the front of the queue
        job_with_lowest_job_score = ready_queue_pop(task_index_least_job_length_score,true);
        ready_queue_add_to_front(&job_with_lowest_job_score);
        
        while(!is_ready_empty())
        {
			PCB current_task_PCB;
			int error_code_load_PCB_TO_CPU;
			int i;

            //task with the lowest job length score runs first
            //in this case, the task with the lowest job length score is the first task in queue
            task_job_length_score = (*readyQueue[0]).job_length_score;
            task_index_least_job_length_score = 0;

            current_task_PCB = (*readyQueue[task_index_least_job_length_score]);
            load_PCB_TO_CPU(current_task_PCB.vm_PC);
            
            error_code_load_PCB_TO_CPU = cpu_run(&current_task_PCB, cpu_quanta_per_program, current_task_PCB.vm_end);

            if(error_code_load_PCB_TO_CPU == 2){
                //the head PCB program has been done, time to reclaim the shell mem
                clean_mem(&readyQueue[task_index_least_job_length_score]->page_tbl);
                ready_queue_pop(task_index_least_job_length_score, true);
                task_job_length_score = MAX_INT;
            }

            if(error_code_load_PCB_TO_CPU == 0){
                //the head PCB program has finished its quanta
                (*readyQueue[task_index_least_job_length_score]).vm_PC = cpu_get_ip(); // update the PC for the PCB
                //Age all the tasks (other than the current executing task) in queue by 1
                for(i = 0; i < QUEUE_LENGTH; i++){
                    //get the lowest job length score
                    if((*readyQueue[i]).vm_start != -1 && (*readyQueue[i]).job_length_score > 0 && i != task_index_least_job_length_score){
                        (*readyQueue[i]).job_length_score -= 1;
                    }
                }
            }
            
            //if the first task job score is not the lowest, 
            //then move the frst task to the end 
            //and the lowest job score task to the front
            for(i = 0; i < QUEUE_LENGTH; i++){
                //get the lowest job length score
                if((*readyQueue[i]).vm_start != -1 && (*readyQueue[i]).job_length_score < task_job_length_score){
                    task_job_length_score = (*readyQueue[i]).job_length_score;
                    task_index_least_job_length_score = i;
                }
            }
            if(task_index_least_job_length_score != 0){
                //pop the task with the lowest job score 
                PCB lowest_job_score_task = ready_queue_pop(task_index_least_job_length_score, true);
                //move the frst task to the end
                PCB first_pcb = ready_queue_pop(0, true);
                ready_queue_add_to_end(&first_pcb);
                //move the lowest job score task to the front
                ready_queue_add_to_front(&lowest_job_score_task);
            }
        
        }
    }

    //clean up
    ready_queue_Empty();
    cpu_empty();

    return error_code;
}

FILE * copy_file_to_backing_store(const char *filename, int *num_lines){
	char backingStore[260];
	int lines = 0;
	FILE *fpSrc, *fpDst;

	fpSrc = fopen(filename, "rt");
	if (!fpSrc)
		return NULL;

	sprintf(backingStore, "./backing_store/%s", filename);
	fpDst = fopen(backingStore, "wt");
	if (!fpDst)
		return (fclose(fpSrc), NULL);

	while (fgets(backingStore, sizeof(backingStore), fpSrc)) {
		fputs(backingStore, fpDst);
		++lines;
	}
	fclose(fpSrc);
	fclose(fpDst);
	
	if (num_lines)
		(*num_lines) = lines;
	sprintf(backingStore, "./backing_store/%s", filename);
	return fopen(backingStore, "rt");
}
