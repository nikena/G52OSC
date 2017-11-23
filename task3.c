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
int consumed = 0;

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

void *threadconsume(void * cindex){
    int consumer_id = *(int*) cindex;
    struct timeval oTimeEnd;
    struct timeval oTimeStart;
    long int responsetime = 0;
    long int turnaroundtime = 0;
    int prevburst = 0;
    struct process *proc = NULL;
    while(consumed<NUMBER_OF_PROCESSES){
        sem_wait(&full);
    
        if(head!=NULL){

            pthread_mutex_lock(&lock);
            proc = getprocess(&head);
            consumed++;
            pthread_mutex_unlock(&lock);
            sem_post(&empty);

            prevburst = proc->iBurstTime;
            simulateSJFProcess(proc, &oTimeStart, &oTimeEnd);
            turnaroundtime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeEnd);
            responsetime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeStart);
            avgresponse += responsetime;
            avgturnaround += turnaroundtime;
            printf("Consumer Id = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n",
             consumer_id, proc->iProcessId, prevburst, proc->iBurstTime, responsetime, turnaroundtime);
            free(proc);

        } else {

            sem_post(&full);
        }
    }
    pthread_exit(NULL);
}

int main(void) {
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_mutex_init(&lock,NULL);

    pthread_t producer;
    pthread_t c[NUMBER_OF_CONSUMERS];

    pthread_create(&producer, NULL, threadproduce, NULL);

    int numofconsumers = 0;
    int consumerid[NUMBER_OF_CONSUMERS];
    for(numofconsumers; numofconsumers < NUMBER_OF_CONSUMERS; numofconsumers ++){
         consumerid[numofconsumers] = numofconsumers;
         pthread_create(&c[numofconsumers], NULL, threadconsume, (void *) &consumerid[numofconsumers]);
    }
    
    pthread_join(producer, NULL);

    for(numofconsumers = 0; numofconsumers < NUMBER_OF_CONSUMERS; numofconsumers ++) {
        pthread_join(c[numofconsumers], NULL);
    }
    printf("Average response time = %f \nAverage turnaround time = %f\n",
            (1.0* avgresponse/NUMBER_OF_PROCESSES), (1.0* avgturnaround/NUMBER_OF_PROCESSES));
    exit(EXIT_SUCCESS);
}
