int interpreter(unsigned int pid, char* command_args[], int args_size);
int help();

//script_pcb

typedef struct script_pcb {
	unsigned int       pid;         //The process PID
	unsigned int       ppid;        //Parent process Id
	struct script_pcb *link_pcb;    //Linked pcb_head
	struct script_pcb *prev_pcb;    //Prev pcb struct
	struct script_pcb *next_pcb;    //Next pcb struct
	char              *script_buf;  //The entire file code buffer, The spot in the Shell memory
	char              *script_ptr;  //The current instruction to execute, NULL means no command any more
	int                num_weight;  //The count of script rows or weight
	unsigned int       num_exec;    //The count of instruction executed
} script_pcb;

extern unsigned int g_max_pid;

//The ready queue contains the PCBs of all the processes currently executing
extern script_pcb * volatile g_pcb_head;

//Code loading
script_pcb * pfork_create(unsigned int pid, const char *script_file, script_pcb *link_pcb);

//Step execute one line
int pfork_step(script_pcb *pcb_ptr);

//Execute sub-processes
int pfork_exec(unsigned int pid, const char *way, int argc, char *argv[]);
int pfork_run(unsigned int pid, const char *script_file);

//Check if reach end of the script
int pfork_eof(script_pcb *pcb_ptr);

//Take a process to execute by using the given way
script_pcb * pfork_take(const char *way);

//Clean up
void pfork_destroy(script_pcb *pcb_ptr);

//Detach
void pfork_detach(script_pcb *pcb_ptr);

//Update count of script rows
void pfork_update(script_pcb *pcb_ptr);

//Get the tail of PCB list
script_pcb * pfork_tail();

//Open an existing process
script_pcb * pfork_open(unsigned int pid, int for_destroy);

//Exit a process by pid
int pfork_exit(unsigned int pid);
