#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>


//implementing my add to the list function here (non decreasing buffer size order)
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

//function to make it to point to the next thing in the list and then free the head after being used
struct process * freememory(struct process *head) {
    struct process *temp = NULL;
        temp = head;
        head = head->oNext;
        free(temp);
        return head;
    }

int main(void) {
    struct process *head = NULL;
    struct process *next = NULL;
    //a struct to store the time when the process ends runing
    struct timeval oTimeEnd;
    struct timeval oTimeStart;
    long int responsetime = 0;
    long int turnaroundtime = 0;
    int avgresponse = 0;
    int avgturnaround = 0;
    int prevburst = 0;

    //generating a process and adding it to the list
    for(int n = 0; n<NUMBER_OF_PROCESSES; n++) {
       next = generateProcess();
       head= add(head,next);
    }
    
    //simulating the processes and calculating the response time and turnaround time, printing it afterwards
    while(head != NULL) {
        prevburst = head->iBurstTime;
        simulateSJFProcess(head, &(oTimeStart), &(oTimeEnd));
        /*calculates the turnaround time*/
        turnaroundtime = getDifferenceInMilliSeconds(head->oTimeCreated, oTimeEnd);
        responsetime = getDifferenceInMilliSeconds(head->oTimeCreated, oTimeStart);
        avgresponse += responsetime;
        avgturnaround += turnaroundtime;
        printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n", head->iProcessId, prevburst, head->iBurstTime, responsetime, turnaroundtime);
        head = freememory(head);
    }
    
    printf("Average response time = %f\nAverage turn around time = %f\n",(1.0 * avgresponse/NUMBER_OF_PROCESSES),(1.0 * avgturnaround/NUMBER_OF_PROCESSES));
    
    exit(EXIT_SUCCESS);
}
