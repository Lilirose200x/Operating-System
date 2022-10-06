#ifndef PCB_H
#define PCB_H

typedef struct PCB{
    int pid;
    int start;
    int end;
    struct PCB *next;
    int current_command;
}pcb;

pcb *make_pcb (int start, int end);
void  setcurrentcommand(pcb* p, int command);
void setnextpcb(pcb *thisp,pcb *nextp);
pcb *getHead();
pcb *getReadyHead();
void addtoReady(pcb *p);
pcb *getPCBfromready(int pid);

#endif