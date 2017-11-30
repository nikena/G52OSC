#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>

//declaring global variables to use in multiple functions 
sem_t empty;
sem_t full;
pthread_mutex_t lock;
struct process *head = NULL;

//function to add to the list according to burst time in non decreasing order 
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

//function that uses temp to save the current head and return it and then make head point to the next thing
struct process *getprocess(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head = (*head)->oNext;
    return temp;
}

/*producer thread, it produces specified number of processes and adds them to the list, uses a semaphore to
 * alert consuemr that there is something produced (and it produces up to the buffer size, uses mutex
 * lock when accessing head to prevent it from being changed or used by other functions
 **/
void *threadproduce(){
    struct process *next = NULL;
    int processcounter = 0;
    while(processcounter < NUMBER_OF_PROCESSES){
        sem_wait(&empty);
        next = generateProcess();
        pthread_mutex_lock(&lock);
        head = add(head,next);
        pthread_mutex_unlock(&lock);
        sem_post(&full);
        processcounter++;
    }
    pthread_exit(NULL);
}

//function to print avg response and turnaround time 
void printavg(int sumresponse, int sumturnaround){
    printf("Average response time = %f \nAverage turnaround time = %f\n",
            (1.0* sumresponse/NUMBER_OF_PROCESSES), (1.0* sumturnaround/NUMBER_OF_PROCESSES));
}
    

/*consumer thread, it consumes the specified number of processes, it wakes up after it has been notified by
 * the producer using a semaphore. The consumer fetches the head of the linked list (using a mutex), simulates 
 * SJFprocess, calculates the response and turnaround time and prints it out. It frees the struct in the end
 */
void *threadconsume(){
    struct timeval oTimeEnd;
    struct timeval oTimeStart;
    long int responsetime = 0; 
    long int turnaroundtime = 0;
    int avgresponse = 0;
    int avgturnaround = 0;
    int prevburst = 0;
    struct process *proc = NULL;

    for(int consumed = 0; consumed < NUMBER_OF_PROCESSES; consumed++){
        sem_wait(&full);
        
        pthread_mutex_lock(&lock);
        proc = getprocess(&head);
        pthread_mutex_unlock(&lock);

        sem_post(&empty);
        
        prevburst = proc->iBurstTime;
        simulateSJFProcess(proc, &oTimeStart, &oTimeEnd);
        turnaroundtime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeEnd);
        responsetime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeStart);
        avgresponse += responsetime;
        avgturnaround += turnaroundtime;
        printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n",
                proc->iProcessId, prevburst, proc->iBurstTime, responsetime, turnaroundtime);
        free(proc);
    }
    printavg(avgresponse, avgturnaround);
    pthread_exit(NULL);
}

int main(void) {
    /* initializing two semaphores, one to restrict making processes above buffer size and one to keep a track
     * of how many processes are there for the consumer to consume and also synchronize them so producer can
     * produce first and then wake up consumer to consume 
     * */
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_mutex_init(&lock,NULL);

    pthread_t producer;
    pthread_t consumer;

    pthread_create(&producer, NULL, threadproduce, NULL);
    pthread_create(&consumer, NULL, threadconsume, NULL);
    
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
   exit(EXIT_SUCCESS);
}
