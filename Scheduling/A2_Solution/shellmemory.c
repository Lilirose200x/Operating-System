#include<stdlib.h>
#include<string.h>
#include<stdio.h>


struct memory_struct{
	char *var;
	char *value;
};

struct memory_struct shellmemory[1000];

// Shell memory functions

void mem_init(){

	int i;
	for (i=0; i<1000; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

// Set key value pair
char * mem_set_value(char *var_in, char *value_in) {
	int i;

	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return (shellmemory[i].value);
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return (shellmemory[i].value);
		} 
	}

	return NULL;
}

//get value based on input key
char *mem_get_value(char *var_in) {
	int i;

	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";
}