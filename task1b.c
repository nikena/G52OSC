#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

//declaring global variables to use in my functions
struct process *head = NULL;
struct process *next = NULL;
struct process *tail = NULL;

long int turnaroundtime = 0;
long int responsetime = 0;
int prevburst = 0;
int newburst = 0;
int state = 0;

//function to add to the tail of the linked list
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

//stuct to separate a process from the head of the list and make it point to the next thing
struct process *getprocess(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head =(*head)->oNext;
    return temp;
}

//Print function that is checking if it is ready, if it has run before (state = 0) and if it is new and finished
void print() {
    if (head->iState == READY) {
        printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response time = %ld\n", head->iProcessId, prevburst, newburst, responsetime);

     } else if(state == 1){
         printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response time = %ld, Turn Around Time = %ld \n", head->iProcessId, prevburst, newburst, responsetime, turnaroundtime);

        } else {
            printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Turn Around Time = %ld \n", head->iProcessId, prevburst, newburst, turnaroundtime);
    }
}

int main(void) {
    struct timeval oTimeEnd;
    struct timeval oTimeStart;
    struct process *temp = NULL;
    int avgresponsetime = 0;
    int avgturnaroundtime =0;

    //generating processes and adding them to the list
    for(int n = 0; n<NUMBER_OF_PROCESSES; n++) {
        next = generateProcess();
        add(&head,next,&tail);  
    }
    
    while(head != NULL) {
        
        //if state = 1, the process is new, if 0 it has run before
        if(head->iState == NEW){
            state = 1;
        } else {
            state = 0;
        }

        prevburst = head->iBurstTime;

        simulateRoundRobinProcess(head, &(oTimeStart), &(oTimeEnd));

        newburst = head-> iBurstTime;
        turnaroundtime = getDifferenceInMilliSeconds(head->oTimeCreated, oTimeEnd);
        responsetime = getDifferenceInMilliSeconds(head->oTimeCreated, oTimeStart);

        print();
            
        //adding to the avgresponse time and avgturnaround time according to different states and also if processes state is ready - adding it back to the list, if finished - freeing the process
        if(head->iState == READY) {
            avgresponsetime += responsetime;
            next = getprocess(&head);
            add(&head, next, &tail);

        } else if (state == 1) {
            avgresponsetime += responsetime;
            avgturnaroundtime += turnaroundtime;
            temp = getprocess(&head);
            free(temp);
            

        } else {
            avgturnaroundtime += turnaroundtime;
            temp = getprocess(&head);
            free(temp);
        }
    }

    printf("Average response time  = %f \nAverage turnaround time = %f\n",(1.0* avgresponsetime/NUMBER_OF_PROCESSES),(1.0*avgturnaroundtime/NUMBER_OF_PROCESSES));
    exit (EXIT_SUCCESS);
}


