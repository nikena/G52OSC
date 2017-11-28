#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

struct process *add(struct process *head, struct process *next) {
    if (head == NULL) {
        return next;

    } else if(next->iBurstTime <= head->iBurstTime) {
        next->oNext=head;
        return next;
    } else {
        struct process *temp = head;
        struct process *prev = NULL;

        while(temp != NULL && temp->iBurstTime <= next->iBurstTime) {
            prev = temp;
            temp = temp->oNext;
        }

        prev->oNext = next;
        next->oNext = temp;
        return head;
    }
}

struct process * freememory(struct process *head) {
    struct process *temp = NULL;
        temp = head;
        head = head->oNext;
        free(temp);
        return head;
    }
/* just an empty struct to pass to simulateprocess*/

int main(void) {
    struct process *head = NULL;
    struct process *next = NULL;
    struct timeval oTimeEnd;
    long int responsetime = 0;
    long int turnaroundtime = 0;
    int avgresponse = 0;
    int avgturnaround = 0;
    int prevburst = 0;

    for(int n = 0; n<NUMBER_OF_PROCESSES; n++) {
       next = generateProcess();
       head= add(head,next);
    }

    while(head != NULL) {
        prevburst = head->iBurstTime;
        simulateSJFProcess(head, &(head->oTimeCreated), &(oTimeEnd));
        /*calculates the turnaround time*/
        turnaroundtime += getDifferenceInMilliSeconds(head->oTimeCreated, oTimeEnd);
        responsetime = turnaroundtime - prevburst;
        avgresponse += responsetime;
        avgturnaround += turnaroundtime;
        printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n", head->iProcessId, prevburst, head->iBurstTime, responsetime, turnaroundtime);
        head = freememory(head);
    }
    
    printf("Average response time = %f\nAverage turn around time = %f\n",(1.0 * avgresponse/NUMBER_OF_PROCESSES),(1.0 * avgturnaround/NUMBER_OF_PROCESSES));
    exit(EXIT_SUCCESS);
}
