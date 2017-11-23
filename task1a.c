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
    int prevburst = 0;

    for(int n = 0; n<NUMBER_OF_PROCESSES; n++) {
       next = generateProcess();
       head= add(head,next);
    }

    while(head != NULL) {
        printf("%d \n", head->iBurstTime);
        prevburst = head->iBurstTime;
        simulateSJFProcess(head, &(head->oTimeCreated), &(oTimeEnd));
        /*calculates the turnaround time*/
        turnaroundtime += getDifferenceInMilliSeconds(head->oTimeCreated, oTimeEnd);
        responsetime = turnaroundtime - prevburst;
        head = freememory(head);
    }
    
    printf("Turnaroundtime is %ld and response time is %ld \n", turnaroundtime, responsetime);
    exit(EXIT_SUCCESS);
}
