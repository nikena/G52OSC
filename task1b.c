#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

struct process *head = NULL;
struct process *next = NULL;
struct process *tail = NULL;
 
long int responsetime = 0;
int avgresponsetime = 0;
long int turnaroundtime = 0;
int avgturnaroundtime =0;
int prevburst = 0;
int newburst = 0;
int state = 0;

void add(struct process **head, struct process *next, struct process **tail) {
    if (*head == NULL) {
        *head = next;
        *tail = next;
 
    } else {
        (*tail)->oNext = next;
        *tail = next;
        (*tail)->oNext = NULL;
    }
}

void freememory(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head = (*head)->oNext;
    free(temp);
}

struct process *returnhead(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head =(*head)->oNext;
    return temp;
}

void print() {
    if (head->iState != FINISHED) {
            avgresponsetime+=responsetime;
            printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response time = %ld\n", head->iProcessId, prevburst, newburst, responsetime);
            next = returnhead(&head);
            add(&head, next, &tail);

        } else if(state == 1){
            state --;
            avgresponsetime +=responsetime;
            avgturnaroundtime += turnaroundtime;
            printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response time = %ld, Turn Around Time = %ld \n", head->iProcessId, prevburst, newburst, responsetime, turnaroundtime);

            freememory(&head);

        } else {
            avgturnaroundtime += turnaroundtime;
            printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Turn Around Time = %ld \n", head->iProcessId, prevburst, newburst, turnaroundtime);
            freememory(&head);
    }
}

int main(void) {
    struct timeval oTimeEnd;

    for(int n = 0; n<NUMBER_OF_PROCESSES; n++) {
        next = generateProcess();
        add(&head,next,&tail);  
    }
    
    while(head != NULL) {

        if(head->iState == NEW){
            state = 1;
        }
        prevburst = head->iBurstTime;
        simulateRoundRobinProcess(head, &(head->oTimeCreated), &(oTimeEnd));
        newburst = head-> iBurstTime;
        turnaroundtime += getDifferenceInMilliSeconds(head->oTimeCreated, oTimeEnd);
        responsetime = turnaroundtime -(prevburst-newburst);
        print();
    }

    printf("Average response time  = %f \nAverage turnaround time = %f\n",(1.0* avgresponsetime/NUMBER_OF_PROCESSES),(1.0*avgturnaroundtime/NUMBER_OF_PROCESSES));
    exit (EXIT_SUCCESS);
}


