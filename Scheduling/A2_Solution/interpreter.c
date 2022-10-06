#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "shellmemory.h"
#include "shell.h"
#include "interpreter.h"

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int badcommand();
int badcommandTooManyTokens();
int badcommandFileDoesNotExist();
int set(char* var, char* value);
int print(char* var);
int simple_run(unsigned int pid, char* script);
int exec(unsigned int pid, char* command_args[], int args_size);
int my_ls();
int echo();

int interpreter(unsigned int pid, char* command_args[], int args_size){
	int i, rc;
	if ( args_size < 1 || args_size > MAX_ARGS_SIZE){
		if (strcmp(command_args[0], "set")==0 && args_size > MAX_ARGS_SIZE) {
			return badcommandTooManyTokens();
		}
		return badcommand();
	}

	for ( i=0; i<args_size; i++){ 
		//strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) 
			return badcommand();
	    return help();
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) 
			return badcommand();
		return quit();
	} else if (strcmp(command_args[0], "set")==0) {
		//set
		char* value;
		char spaceChar;
		int i;
		if (args_size < 3) 
			return badcommand();
		value = (char*)calloc(1,150);
		spaceChar = ' ';
		for(i = 2; i < args_size; i++){
			strncat(value, command_args[i], 30);
			if(i < args_size-1){
				strncat(value, &spaceChar, 1);
			}
		}
		return set(command_args[1], value);
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) 
			return badcommand();
		return print(command_args[1]);
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) 
			return badcommand();
		rc = pfork_run(pid, command_args[1]);
		if (rc)
			return badcommandFileDoesNotExist();
		return 0;
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size > 2) 
			return badcommand();
		return my_ls();
	}else if (strcmp(command_args[0], "echo")==0) {
		if (args_size > 2) 
			return badcommand();
		return echo(command_args[1]);
	}
	else if (strcmp(command_args[0], "exec")==0) {
		if (args_size > 5 || args_size<=2) 
			return badcommand();
		return exec(pid, command_args, args_size);
	} 
	else 
		return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with Bye!\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");
	exit(0);
	return 0;
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 2;
}

int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommandexec(){
	printf("%s\n", "Unknown Command in exec");
	return 1;
}

int badcommandduplicate(){
	printf("%s\n", "Duplicate command in arguments");
	return 1;
}

int set(char* var, char* value){
	//char *link = "=";
	//char buffer[1000];
	//strcpy(buffer, var);
	//strcat(buffer, link);
	//strcat(buffer, value);
	mem_set_value(var, value);
	return 0;
}

int print(char* var){
	printf("%s\n", mem_get_value(var)); 
	return 0;
}

int simple_run(unsigned int pid, char* script){
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	fgets(line,999,p);
	while(1){
		errCode = parseInput(0, line);	// which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)){
			break;
		}
		fgets(line,999,p);
	}

    fclose(p);
	return errCode;
}

int my_ls(){
	int errCode = system("ls | sort");
	return errCode;
}

int echo(char* var){
	if(var[0] == '$'){
		var++;
		printf("%s\n", mem_get_value(var)); 
	}else{
		printf("%s\n", var); 
	}
	return 0; 
}

int exec(unsigned int pid, char* command_args[], int args_size) {
	//NOTE: command_args[0] == "exec"
	int rc;
	if (strcmp(command_args[args_size - 1], "FCFS") 
		&& strcmp(command_args[args_size - 1], "SJF") 
		&& strcmp(command_args[args_size - 1], "RR") 
		&& strcmp(command_args[args_size - 1], "AGING"))
	{
		return badcommandexec();
	}
	else if (args_size==4 && strcmp(command_args[1],command_args[2])==0){
    return badcommandduplicate();
    }
    else if (args_size==5 && (strcmp(command_args[1],command_args[2])==0
    ||strcmp(command_args[2],command_args[3])==0
    ||strcmp(command_args[3],command_args[1])==0)){
    return badcommandduplicate();
    }
	else if (3 == args_size) {
		//Same to perform run()
		rc = pfork_run(pid, command_args[1]);
		if (rc)
			return badcommandFileDoesNotExist();
		return 0;
	}

	rc = pfork_exec(pid, command_args[args_size - 1], (args_size - 2), (command_args + 1));
	if (rc)
		return badcommandFileDoesNotExist();
	return 0;
}

//script_pcb

unsigned int g_max_pid = 0;
script_pcb * volatile g_pcb_head = NULL;

script_pcb * pfork_create(unsigned int pid, const char *script_file, script_pcb *link_pcb) {
	int rc;
	long len;
	FILE *fp;
	char *buf;
	script_pcb *pcb_ptr;
	char var[16];
	unsigned int new_pid = (g_max_pid + 1);
	
	//Load script code
	fp = fopen(script_file, "rb");
	if (!fp)
		return NULL;
	rc = fseek(fp, 0, SEEK_END);
	if (rc)
		return (fclose(fp), NULL);
	len = ftell(fp);
	rc = fseek(fp, 0, SEEK_SET);
	if (rc)
		return (fclose(fp), NULL);

	buf = (char *)malloc(len + 1);
	if (!buf)
		return (fclose(fp), NULL);
	rc = fread(buf, 1, len, fp);
	fclose(fp);
	if (rc != len)
		return (free(buf), NULL);
	buf[len] = '\0';

	//Allocate new PCB struct
	pcb_ptr = (script_pcb *)malloc(sizeof(script_pcb));
	if (!pcb_ptr)
		return (free(buf), NULL);

	//Store the entire source code into OS Shell memory
	sprintf(var, "$pid_%u", new_pid);
	pcb_ptr->script_buf = mem_set_value(var, buf);
	free(buf);
	if (!pcb_ptr->script_buf)
		return (free(pcb_ptr), NULL);

	pcb_ptr->script_ptr = pcb_ptr->script_buf;
	pcb_ptr->num_weight = -1;
	pcb_ptr->num_exec = 0;
	pcb_ptr->pid = new_pid;
	pcb_ptr->ppid = pid;
	pcb_ptr->prev_pcb = NULL;
	pcb_ptr->next_pcb = g_pcb_head;
	if (g_pcb_head) {
		g_pcb_head->prev_pcb = pcb_ptr;
		pcb_ptr->link_pcb = g_pcb_head->link_pcb;
	} else {
		pcb_ptr->link_pcb = link_pcb;
	}
	g_pcb_head = pcb_ptr;

	//Update next new_pid
	g_max_pid = new_pid;
	return pcb_ptr;
}

int pfork_step(script_pcb *pcb_ptr) {
	char *p, *s;
	if (!pcb_ptr->script_ptr)
		return -1;
	s = pcb_ptr->script_ptr;
	p = strchr(s, '\n');
	if (p) {
		if((p != s) && ('\r' == p[-1]))
			p[-1] = '\0';
		else
			p[0] = '\0';
		pcb_ptr->script_ptr = (p + 1);
	} else {
		pcb_ptr->script_ptr = NULL;
	}
	pcb_ptr->num_exec++;
	if (s[0])
		return parseInput(pcb_ptr->pid, s);
	else
		return 0;
}

int pfork_run(unsigned int pid, const char *script_file) {
	//Loading script code and create a new process struct
	int rc;
	script_pcb *pcb_ptr = pfork_create(pid, script_file, NULL);
	if (!pcb_ptr)
		return -1;

	//Only one sub-process, execute to the end
	while (!pfork_eof(pcb_ptr)) {

		rc = pfork_step(pcb_ptr);
		if (rc) {
			//If an error occurs while executing the script due a command syntax error, 
			//then the error is displayed, and the script continues executing.
		}
	}

	//Destroy the process
	pfork_destroy(pcb_ptr);
	return 0;
}

int pfork_exec(unsigned int pid, const char *way, int argc, char *argv[]) {
	int ind, rc;
	script_pcb *pcb_ptr;
	script_pcb *link_pcb;

	//Create sub-processes
	link_pcb = g_pcb_head;
	g_pcb_head = NULL;
	for (ind = (argc - 1); ind >= 0; ind--) {
		pcb_ptr = pfork_create(pid, argv[ind], link_pcb);
		if (!pcb_ptr) {
			//If there is a code loading error, then no programs run
			while (g_pcb_head)
				pfork_destroy(g_pcb_head);

			g_pcb_head = link_pcb;
			return -1;
		}
	}

	//Executes up to 3 concurrent programs, according to a given scheduling policy
	for (;;) {
		pcb_ptr = pfork_take(way);
		if (!pcb_ptr)
			break;
		if (pfork_eof(pcb_ptr))
			pfork_destroy(pcb_ptr);
		else
			rc = pfork_step(pcb_ptr);
	}

	g_pcb_head = link_pcb;
	return 0;
}

int pfork_eof(script_pcb *pcb_ptr) {
	return (pcb_ptr->script_ptr ? 0 : 1);
}

script_pcb * pfork_take(const char *way) {
	if (!g_pcb_head)
		return NULL;
	if (!g_pcb_head->next_pcb)
		return g_pcb_head;

	if (strcmp(way, "FCFS") == 0) {
		return g_pcb_head;
	}
	else if (strcmp(way, "SJF") == 0) {
		int min_weight = (1 << 30);
		script_pcb *min_pcb_ptr = NULL;
		script_pcb *pcb_ptr;
		for (pcb_ptr = g_pcb_head; pcb_ptr; pcb_ptr = pcb_ptr->next_pcb) {
			if (pcb_ptr->num_weight < 0)
				pfork_update(pcb_ptr);
			if (pcb_ptr->num_weight < min_weight) {
				min_weight = pcb_ptr->num_weight;
				min_pcb_ptr = pcb_ptr;
			}
		}
		return min_pcb_ptr;
	}
	else if (strcmp(way, "RR") == 0) {
		script_pcb *pcb_ptr;
		if (g_pcb_head->num_exec < 2)
			return g_pcb_head;
		pcb_ptr = pfork_tail();
		pcb_ptr->next_pcb = g_pcb_head;
		g_pcb_head->prev_pcb = pcb_ptr;
		g_pcb_head->num_exec = 0;
		pcb_ptr = g_pcb_head->next_pcb;
		g_pcb_head->next_pcb = NULL;
		pcb_ptr->prev_pcb = NULL;
		g_pcb_head = pcb_ptr;
		return g_pcb_head;
	}
	else if (strcmp(way, "AGING") == 0) {
		int min_weight;
		script_pcb *min_pcb_ptr;
		script_pcb *pcb_ptr;
		if (g_pcb_head->num_weight < 0) {
			//First time select, same as SJF
			min_pcb_ptr = NULL;
			min_weight = (1 << 30);
			for (pcb_ptr = g_pcb_head; pcb_ptr; pcb_ptr = pcb_ptr->next_pcb) {
				if (pcb_ptr->num_weight < 0)
					pfork_update(pcb_ptr);
				if (pcb_ptr->num_weight < min_weight) {
					min_weight = pcb_ptr->num_weight;
					min_pcb_ptr = pcb_ptr;
				}
			}
			return min_pcb_ptr;
		}
		else {
			min_pcb_ptr = g_pcb_head;
			min_weight = g_pcb_head->num_weight;
			for (pcb_ptr = g_pcb_head->next_pcb; pcb_ptr; pcb_ptr = pcb_ptr->next_pcb) {
				if (pcb_ptr->num_weight > 0)
					pcb_ptr->num_weight--;
				if (pcb_ptr->num_weight < min_weight) {
					min_weight = pcb_ptr->num_weight;
					min_pcb_ptr = pcb_ptr;
				}
				else if ((pcb_ptr->num_weight == min_weight) && (min_pcb_ptr != g_pcb_head)) {
					min_weight = pcb_ptr->num_weight;
					min_pcb_ptr = pcb_ptr;
				}
			}
			if (g_pcb_head == min_pcb_ptr)
				return g_pcb_head;
			if (0 == min_weight)
				return pfork_tail();  //Same as rev-FCFS
			min_pcb_ptr->prev_pcb->next_pcb = min_pcb_ptr->next_pcb;
			if (min_pcb_ptr->next_pcb)
				min_pcb_ptr->next_pcb->prev_pcb = min_pcb_ptr->prev_pcb;
			min_pcb_ptr->prev_pcb = NULL;
			min_pcb_ptr->next_pcb = g_pcb_head;
			g_pcb_head->prev_pcb = min_pcb_ptr;
			g_pcb_head = min_pcb_ptr;
			return g_pcb_head;
		}
	}
	else {
		return NULL;
	}
}

void pfork_destroy(script_pcb *pcb_ptr) {
	pfork_detach(pcb_ptr);
	free(pcb_ptr);
}

void pfork_detach(script_pcb *pcb_ptr) {
	if (pcb_ptr->prev_pcb)
		pcb_ptr->prev_pcb->next_pcb = pcb_ptr->next_pcb;
	else
		g_pcb_head = pcb_ptr->next_pcb;
	if (pcb_ptr->next_pcb)
		pcb_ptr->next_pcb->prev_pcb = pcb_ptr->prev_pcb;
}

void pfork_update(script_pcb *pcb_ptr) {
	int rows = 1;
	const char *buf_ptr = pcb_ptr->script_buf;
	for (; (*buf_ptr); ++buf_ptr) {
		if ('\n' == (*buf_ptr))
			++rows;
	}
	pcb_ptr->num_weight = rows;
}

script_pcb * pfork_tail() {
	script_pcb *pcb_ptr;
	for (pcb_ptr = g_pcb_head; pcb_ptr->next_pcb; ) {
		pcb_ptr = pcb_ptr->next_pcb;
	}
	return pcb_ptr;
}

script_pcb * pfork_open(unsigned int pid, int for_destroy) {
	script_pcb *link_pcb;
	script_pcb *pcb_ptr;
	if (pid > g_max_pid)
		return NULL;
	for (link_pcb = g_pcb_head; link_pcb; link_pcb = link_pcb->link_pcb) {
		for (pcb_ptr = link_pcb; pcb_ptr; pcb_ptr = pcb_ptr->next_pcb) {
			if (pid == pcb_ptr->pid) {
				if (for_destroy && (link_pcb != g_pcb_head))
					return NULL;
				return pcb_ptr;
			}
		}
	}
	return NULL;
}

int pfork_exit(unsigned int pid) {
	script_pcb *pcb_ptr = pfork_open(pid, 1);
	if (!pcb_ptr)
		return -1;
	pfork_destroy(pcb_ptr);
	return 0;
}
