#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

sem_t empty;
sem_t full;
pthread_mutex_t lock;

struct process *tail = NULL;
struct process *head = NULL;

struct process *headq1 = NULL;
struct process *headq2 = NULL;
struct process *tailq1 = NULL;
struct process *tailq2 = NULL;

int avgresponse = 0;
int avgturnaround = 0;

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

struct process *getprocess(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head = (*head)->oNext;
    temp->oNext = NULL;
    return temp;
}


void print (int processid, int istate, int previous, int new, int state, int id, long int turnaround, long int response){
    if (istate != FINISHED) {
        printf("Consumer Id = %d, Process Id = %d, Previous Burst Time =%d, New Burst Time = %d, Response time = %ld\n", id, processid, previous, new, response);

    } else if (state == 1){
        printf("Consumer Id = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d,  Response time = %ld, Turn Around time = %ld\n", id, processid, previous, new, response, turnaround);
        
    } else {
        printf("Consumer Id = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Turn Around Time = %ld\n", id, processid, previous, new, turnaround);
    }
}

void *threadproduce(){
    struct process *next = NULL;
    int a = 0;
    while(a < NUMBER_OF_PROCESSES){
        sem_wait(&empty);
        next = generateProcess();
        pthread_mutex_lock(&lock);
        add(&head,next,&tail);
        pthread_mutex_unlock(&lock);
        a++;
        sem_post(&full);
    }
    pthread_exit(NULL);
}
 
void *threadconsume(void * cindex){
    int processid = 0;
    int prevburst = 0; 
    int newburst =0;
    int state = 0;
    int istate = 0;
    int consumer_id = *(int*) cindex;
    long int responsetime = 0;
    int sumresponse = 0;
    long int turnaroundtime = 0;
    int sumturnaround = 0;

    struct timeval oTimeEnd;
    struct timeval oTimeStart;
    struct process *proc = NULL;
    while(1){
        sem_wait(&full);
    

            pthread_mutex_lock(&lock);
            if(head == NULL){
                pthread_mutex_unlock(&lock);
                avgresponse += sumresponse;
                avgturnaround += sumturnaround;
                sem_post(&full);
                pthread_exit(NULL);
            }
            proc = getprocess(&head);
            pthread_mutex_unlock(&lock);

            if(proc->iState == NEW) {
                state = 1;
            } else {
                state = 0;
            }
            
            processid = proc->iProcessId;
            prevburst = proc->iBurstTime;
            simulateBlockingRoundRobinProcess(proc, &oTimeStart, &oTimeEnd);
            newburst = proc->iBurstTime;
            istate = proc->iState;
            turnaroundtime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeEnd);
            responsetime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeStart);
            print(processid, istate, prevburst, newburst, state, consumer_id, turnaroundtime, responsetime);

            if(proc->iState != FINISHED){
                sumresponse += responsetime;
            } else if(state == 1) {
                sumresponse += responsetime;
                sumturnaround += turnaroundtime;
            } else {
                sumturnaround += turnaroundtime;
            }

            if(proc->iState != READY) {
               state--;
               pthread_mutex_lock(&lock);
               add(&head, proc, &tail);
               sem_post(&full);
               pthread_mutex_unlock(&lock);

            } else if(proc->iState == BLOCKED) {
                sem_post(&empty);    
                if(proc->iEventType == 1) {
                    pthread_mutex_lock(&lock);
                    add(&headq1, proc, &tailq1);
                    pthread_mutex_unlock(&lock);
                } else {
                    pthread_mutex_lock(&lock);
                    add(&headq2, proc, &tailq2);
                    pthread_mutex_unlock(&lock);
                }

            } else if(proc->iState == FINISHED) {
                sem_post(&empty);
                free(proc);
            }

    }
}

int main(void) {

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_mutex_init(&lock,NULL);

    pthread_t producer;
    pthread_t c[NUMBER_OF_CONSUMERS];
    pthread_t eventManager;

    pthread_create(&producer, NULL, threadproduce, NULL);

    int consumerid[NUMBER_OF_CONSUMERS];
    for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        consumerid[i] = i;
        pthread_create(&(c[i]), NULL, threadconsume, (void *) &consumerid[i]);
    }
    
    pthread_create(&eventManager, NULL, eventQueues, NULL);

    pthread_join(producer, NULL);

    sem_post(&full);   

    for(int n = 0; n < NUMBER_OF_CONSUMERS; n++) {
        pthread_join(c[n], NULL);
    }

    pthread_join(eventManager, NULL);

    printf("Average response time = %f \nAverage turnaround time = %f\n",
            (1.0* avgresponse/NUMBER_OF_PROCESSES), (1.0* avgturnaround/NUMBER_OF_PROCESSES));
    exit(EXIT_SUCCESS);
}
