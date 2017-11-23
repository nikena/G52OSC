#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>

sem_t empty;
sem_t full;
pthread_mutex_t lock;
struct process *head = NULL;
int avgresponse = 0;
int avgturnaround = 0;

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

struct process *getprocess(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head = (*head)->oNext;
    return temp;
}

void *threadproduce(){
    struct process *next = NULL;
    int processcounter = 0;
    while(processcounter < NUMBER_OF_PROCESSES){
        sem_wait(&empty);
        next = generateProcess();
        pthread_mutex_lock(&lock);
        head = add(head,next);
        pthread_mutex_unlock(&lock);
        processcounter++;
        sem_post(&full);
    }
    pthread_exit(NULL);
}

void *threadconsume(){
    struct timeval oTimeEnd;
    struct timeval oTimeStart;
    long int responsetime = 0;
    long int turnaroundtime = 0;
    int prevburst = 0;
    struct process *proc = NULL;
    for(int consumed = 0; consumed < NUMBER_OF_PROCESSES; consumed++){
        sem_wait(&full);

        pthread_mutex_lock(&lock);
        proc = getprocess(&head);
        pthread_mutex_unlock(&lock);

        prevburst = proc->iBurstTime;
        simulateSJFProcess(proc, &oTimeStart, &oTimeEnd);
        turnaroundtime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeEnd);
        responsetime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeStart);
        avgresponse += responsetime;
        avgturnaround += turnaroundtime;
        printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n",
                proc->iProcessId, prevburst, proc->iBurstTime, responsetime, turnaroundtime);
        free(proc);
        sem_post(&empty);
    }
    pthread_exit(NULL);
}

int main(void) {
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_mutex_init(&lock,NULL);

    pthread_t producer;
    pthread_t consumer;

    pthread_create(&producer, NULL, threadproduce, NULL);
    pthread_create(&consumer, NULL, threadconsume, NULL);
    
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    printf("Average response time = %f \nAverage turnaround time = %f\n",
            (1.0* avgresponse/NUMBER_OF_PROCESSES), (1.0* avgturnaround/NUMBER_OF_PROCESSES));
    exit(EXIT_SUCCESS);
}
