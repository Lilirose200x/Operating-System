#include<stdio.h>
#include<stdlib.h>
#include "pcb.h"

pcb *head = NULL;
pcb *tail = NULL;

pcb *make_pcb (int start, int end) {
    pcb *thisPCB= (pcb*) malloc ( sizeof ( pcb) ) ;
    thisPCB->pid = rand();  //TODO: pid必须保证唯一性【Make sure each process has a unique PID.】，不能使用随机数！
	thisPCB->start = start ;
	thisPCB->end = end ;
	thisPCB->next = NULL ;
    thisPCB->current_command=0;
    return thisPCB ;
}

void setcurrentcommand(pcb *p, int command) {
    p->current_command=command;
}

void setnextpcb(pcb *thisp, pcb *nextp){
    thisp ->next=nextp;
}
pcb *getHead(){
    return head;
}

void addtoReady(pcb *p){
    if(!head){
        head = p;
        tail = p;
    }
    else{
        tail->next = p;
        tail = p;
    }
}

pcb *getReadyHead(){
    pcb *out;

    if(!head)
        return NULL;

    out = head;
    head = head->next;
    out->next = NULL;

    if(!head)
        tail = NULL;
    return out;
}

int getcurrentcommand(pcb *p){
    return p->current_command;
}
pcb * getPCBfromready(int pid){
     pcb *out;
     if(!head)
        return NULL; 
    out=head;
    if(out->pid=pid){
    head = head->next;
    out->next = NULL;

    if(!head)
        tail = NULL;
    return out;  
    }else{
    out=head->next;
    if(out->pid=pid){
    head->next=head->next->next;
    out->next = NULL;
    return out;  
    }else{
        out=head->next->next;
        if(out->pid=pid){
            head->next->next=NULL;
            tail=head->next;
            out->next=NULL;
            return out;
        }else return NULL;
     }
    
    }

}