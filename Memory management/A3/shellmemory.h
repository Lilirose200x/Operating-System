#include<stdlib.h>
#include<stdio.h>
#include "pcb.h"

#ifdef WIN32
#define strdup _strdup
#define access _access
#pragma warning(disable: 4996)
#endif

#define FRAME_LINE_COUNT 3
#define FRAME_INIT_COUNT 2

#define SHELL_MEM_LENGTH     1000
#define SHELL_MEM_STOCK_OFF  (100*FRAME_LINE_COUNT)

#ifdef _DEBUG
#define FRAME_MEM_LENGTH  18
#define VAR_MEM_LENGTH    10
#endif

void mem_init();
void var_mem_init();
char *mem_get_value(char *var);
char* mem_get_value_by_line(int line);
unsigned int mem_get_last_used_by_line(int line);
char *mem_get_var_by_line(int line);
void mem_none_var_by_line(int line);
void mem_set_last_used_by_line(int line, unsigned int time);
void mem_set_value(char *var, char *value);
void clean_mem(PAGE_TABLE *page_tbl);
int  add_file_to_mem(FILE* fp, int* pStart, int* pEnd, char* fileID, unsigned int limit, int bSwapPagefile);
void victim_page_fault(int phy_PC);
void raise_page_fault(PCB *pPCB, int vm_PC);
void backing_store_init();
void backing_store_term();
