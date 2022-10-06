#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 100;

int help();
int quit();
int badcommand();
int set(char* var, char** value_ptr, int value_count);
int echo(char** value_ptr, int value_count);
int print(char* var);
int run(char* script);
int my_ls(void);
int badcommandFileDoesNotExist();
int badcommandTooManyTokens();

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){
	int i;
	int count;

	if ( args_size < 1 || args_size > MAX_ARGS_SIZE){
		return badcommand();
	}

	for ( i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		if (args_size < 3) 
			return badcommand();
		count = (args_size - 2);
		if(count > 5)
			return badcommandTooManyTokens();
		return set(command_args[1], &command_args[2], count);

	} else if (strcmp(command_args[0], "echo")==0) {
		//echo
		count = (args_size - 1);
		return echo(&command_args[1], count);
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
	
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size != 1) return badcommand();
	    return my_ls();
	} else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 4;
}

int set(char* var, char** value_ptr, int value_count){
	int i;
	//char *link = "=";
	char buffer[1000];
	strcpy(buffer, value_ptr[0]);

	for(i = 1; i < value_count; ++i){
		strcat(buffer, " ");
		strcat(buffer, value_ptr[i]);
	}

	mem_set_value(var, buffer);

	return 0;
}

int echo(char** value_ptr, int value_count){
	char *var_value;
	int i;
	for (i = 0; i < value_count; ++i){
		if ('$' == value_ptr[i][0]){
			var_value = mem_get_value_v2((value_ptr[i] + 1), NULL);
			if (var_value)
				printf("%s\n", var_value);
		}
		else {
			printf("%s\n", value_ptr[i]);
		}
	}
	return 0;
}

int print(char* var){
	printf("%s\n", mem_get_value(var)); 
	return 0;
}

int run(char* script){
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	fgets(line,999,p);
	while(1){
		errCode = parseInput(line);	// which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)){
			break;
		}
		fgets(line,999,p);
	}

    fclose(p);

	return errCode;
}
int my_ls(void){
system("ls | cat\n");
return 0;
}