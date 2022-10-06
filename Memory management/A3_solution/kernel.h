#ifndef kernel_H
#define kernel_H

#include "pcb.h"

void ready_queue_initialize();

int get_scheduling_policy_number(char* policy);

int myinit(const char *filename);
int scheduler();

FILE * copy_file_to_backing_store(const char *filename, int *num_lines);

#endif
