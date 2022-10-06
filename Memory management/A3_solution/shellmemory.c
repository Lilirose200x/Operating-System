#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include "shellmemory.h"
#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#else
#include<unistd.h>
#include<sys/types.h> 
#include<sys/stat.h>
#endif

struct memory_struct{
	char *var;
	char *value;
	unsigned int last_used;
};

struct memory_struct shellmemory[SHELL_MEM_LENGTH];
struct memory_struct varmemory[VAR_MEM_LENGTH];
// Shell memory functions

void mem_init(){
	int i;
	for (i=0; i<SHELL_MEM_LENGTH; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

void var_mem_init(){
	int i;
	for (i=0; i<VAR_MEM_LENGTH; i++){		
		varmemory[i].var = "none";
		varmemory[i].value = "none";
	}
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {

	int i;

	for (i=0; i<VAR_MEM_LENGTH; i++){
		if (strcmp(varmemory[i].var, var_in) == 0){
			varmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<VAR_MEM_LENGTH; i++){
		if (strcmp(varmemory[i].var, "none") == 0){
			varmemory[i].var = strdup(var_in);
			varmemory[i].value = strdup(value_in);
			return;
		} 
	}
}

//get value based on input key
char *mem_get_value(char *var_in) {
	int i;
	for (i=0; i<SHELL_MEM_LENGTH; i++){
		if (strcmp(varmemory[i].var, var_in) == 0){
			return strdup(varmemory[i].value);
		} 
	}
	return "Variable does not exist";
}

char* mem_get_value_by_line(int line){
	return shellmemory[line].value;
}

unsigned int mem_get_last_used_by_line(int line){
	return shellmemory[line].last_used;
}

char * mem_get_var_by_line(int line){
	return shellmemory[line].var;
}

void mem_none_var_by_line(int line){
	shellmemory[line].var = strdup("none");
	shellmemory[line].value[0] = 0;
}

void mem_set_last_used_by_line(int line, unsigned int time){
	shellmemory[line].last_used = time;
}

void clean_mem(PAGE_TABLE *page_tbl){

	destroyPageTable(page_tbl);
}

int add_file_to_mem(FILE* fp, int* pStart, int* pEnd, char* fileID, unsigned int limit, int bSwapPagefile)
{
    char line[100];
    size_t i, j;
    int error_code = 0;
	bool hasSpaceLeft = false;
	unsigned int noneBlock = 0;
	size_t blockStart = SHELL_MEM_STOCK_OFF;
	size_t blockEnd = SHELL_MEM_LENGTH;
	
	if (1) {
		blockStart = ((SHELL_MEM_LENGTH - FRAME_MEM_LENGTH) / FRAME_LINE_COUNT * FRAME_LINE_COUNT);
		blockEnd = blockStart + FRAME_MEM_LENGTH;
	}

    for (i = blockStart; i < blockEnd; ){
        if(strcmp(shellmemory[i].var,"none") == 0){
			++noneBlock;
			i++;
        }
		else {
			noneBlock = 0;
			i = ( ((i / FRAME_LINE_COUNT) + 1) * FRAME_LINE_COUNT);
		}
		if (limit == noneBlock) {
			*pStart = (int)(i - limit);
			i = *pStart;
			hasSpaceLeft = true;
			break;
		}
    }

	//shell memory is full
	if(hasSpaceLeft == 0){
		if (bSwapPagefile && (FRAME_LINE_COUNT == limit)){
			unsigned int minUsed = (1 << 31);
			size_t minIdx = -1;
			char *minVar;
			for (i = blockStart; (i + FRAME_LINE_COUNT) <= blockEnd; i += FRAME_LINE_COUNT){
				if (mem_get_last_used_by_line(i) < minUsed){
					minUsed = mem_get_last_used_by_line(i);
					minIdx = i;
				}
			}

			victim_page_fault(minIdx);

			minVar = mem_get_var_by_line(minIdx);
			for(i=0; i<FRAME_LINE_COUNT; ++i){
				mem_none_var_by_line(minIdx + i);
			}
			*pStart = (int)(minIdx);
			i = *pStart;
		}
		else {
			error_code = 21;
			return error_code;
		}
	}
    
    for (j = i; j < blockEnd; j++){
        if ((0 == limit) || feof(fp))
        {
            *pEnd = (int)j-1;
            break;
        }else{
            fgets(line, 999, fp);
			shellmemory[j].var = strdup(fileID);
            shellmemory[j].value = strdup(line);
			limit--;
        }
    }

	//no space left to load the entire file into shell memory
	//if(!feof(fp)){
	//	error_code = 21;
	//	//clean up the file in memory
	//	for(j = 1; i <= blockEnd; i ++){
	//		shellmemory[j].var = "none";
	//		shellmemory[j].value = "none";
	//   	}
	//	return error_code;
	//}
    return error_code;
}

void raise_page_fault(PCB *pPCB, int vm_PC) {
#ifdef _WIN32
	void __stdcall OutputDebugStringA(const char *);
	OutputDebugStringA("[raise_page_fault]\n");
#else
	//Do nothing else
#endif
}

void victim_page_fault(int phy_PC) {
	int i;
	char *p;
	if (phy_PC < 0) {
		//printf("[Page fault] process=%s, vm_addr=%u\n", pPCB->pid, vm_PC);
		return;
	}

	printf("Page fault! Victim page contents:\n");

	//target_frame = find_victim_frame();
	//print_record();
	//printf("victim frame is %d\n", target_frame);
	for (i = 0; i < FRAME_LINE_COUNT; i++) {
		int position = (phy_PC + i);
		if (strcmp(mem_get_value_by_line(position), "none") == 0) {
			break;
		}
		p = mem_get_value_by_line(position);
		printf((('\n' == p[strlen(p)-1]) ? "%s" : "%s\n"), p);
	}

	printf("End of victim page contents\n");
	//prev_victim_frame = target_frame;
}

void backing_store_init()
{
	int rc;
#ifdef WIN32
	rc = mkdir("./backing_store");
#else
	rc = mkdir("./backing_store", 0775);
#endif	
}

void backing_store_term()
{
	int rc;
#ifdef WIN32
	rc = rmdir("./backing_store");
	if (rc)
		rc = (int)(INT_PTR)ShellExecuteA(NULL, NULL, "cmd.exe", "/c rd /s /q .\\backing_store", NULL, SW_HIDE);
#else
	rc = rmdir("./backing_store");
	if (rc)
		rc = system("rm -rf ./backing_store/");
#endif
}
