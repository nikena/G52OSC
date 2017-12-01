#include "coursework.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

sem_t empty;
sem_t full;

//initializing 4 mutexes, 3 for each head of the three lists and 1 for printing
pthread_mutex_t lock;
pthread_mutex_t q1;
pthread_mutex_t q2;
pthread_mutex_t p;

struct process *tail = NULL;
struct process *head = NULL;

struct process *headq1 = NULL;
struct process *headq2 = NULL;
struct process *tailq1 = NULL;
struct process *tailq2 = NULL;

int avgresponse = 0;
int avgturnaround = 0;

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

//function using temp to save the current head and separate it from the list, making the next element the new head
struct process *getprocess(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head = (*head)->oNext;
    temp->oNext = NULL;
    return temp;
}

/* void function to print, it prints the consumer id, process id, previous and new burst for everything, after that according to different states it prints additional response time or turnaround time. When process is new (state = 1) it prints only the response time, when process is finished, it prints turnaround time
 */
void print (int processid, int istate, int previous, int new, int state, int id, long int turnaround, long int response){
    printf("Consumer Id = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d", id, processid, previous, new);
    if (state == 1) {
        printf(", Response time = %ld", response);
    }
    else if (istate == FINISHED){
        printf(", Turn Around time = %ld", turnaround);
    } 
    
    printf("\n");
}

/*producer thread that produces specified number of processes, adds them to the list (using mutex when accessing
* global struct head) and does a sem_post to wake up consumer to say there is something ready for consuming
*/ 
void *threadproduce(){
    struct process *next = NULL;
    int a = 0;
    while(a < NUMBER_OF_PROCESSES){
        sem_wait(&empty);
        next = generateProcess();
        pthread_mutex_lock(&lock);
        add(&head,next,&tail);
        pthread_mutex_unlock(&lock);
        sem_post(&full);
        a++;
    }
    pthread_exit(NULL);
}

/*consumer thread that gets woken up by producer, checks if head is null and if it is, it then checks the other
 * two linked lists which belong to the eventmanager thread, if they are all null then it adds up the avgresponse
 * and avgturnaround time, notifies the next consumer that everything is done and he can exit and exits. If head
 * is not null, it gets the head of the list, assigns state to it, if it is new then state =1, if not new
 *  then state = 0, simulates RR on it, gets turnaround and responsetime, uses the print function in a mutex
 *   (or it might print different bits of a few consumers at the same time). Then depending on different states
 *   it either add it back to the list, or blocks it and sends it to the corresponding event queue 1 or 2 or if 
 *   it finishes - frees it 
 */ 
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
            pthread_mutex_lock(&q1);
            pthread_mutex_lock(&q2);
            if(headq1 == NULL && headq2 == NULL){
                pthread_mutex_unlock(&q1);
                pthread_mutex_unlock(&q2);
                /*saw this in a lecture, i use it so we can only add to avgresponse time and avg turnaround time 
                *number of processes times, not every time a consumer has ran and done something
                */
                avgresponse += sumresponse;
                avgturnaround += sumturnaround;
                sem_post(&full);
                pthread_exit(NULL);
            }
        }

        proc = getprocess(&head);
        pthread_mutex_unlock(&lock);
        pthread_mutex_unlock(&q1);
        pthread_mutex_unlock(&q2);

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
        pthread_mutex_lock(&p);
        print(processid, istate, prevburst, newburst, state, consumer_id, turnaroundtime, responsetime);
        pthread_mutex_unlock(&p);

        if(state == 1){
            sumresponse += responsetime;
        } else if(proc->iState == FINISHED){
            sumturnaround += turnaroundtime;
        }

        if(proc->iState == READY) {
           state--;
           pthread_mutex_lock(&lock);
           add(&head, proc, &tail);
           sem_post(&full);
           pthread_mutex_unlock(&lock);

        } else if(proc->iState == BLOCKED) {
            sem_post(&empty);    
            if(proc->iEventType == 1) {
                pthread_mutex_lock(&p);
                printf("Process %d blocked on event type 1\n", processid);
                pthread_mutex_unlock(&p);
                pthread_mutex_lock(&lock);
                add(&headq1, proc, &tailq1);
                pthread_mutex_unlock(&lock);
            } else {
                pthread_mutex_lock(&p);
                printf("Process %d blocked on event type 2\n", processid);
                pthread_mutex_unlock(&p);
                pthread_mutex_lock(&lock);
                add(&headq2, proc, &tailq2);
                pthread_mutex_unlock(&lock);
            }

        } else {
            sem_post(&empty);
            free(proc);
        }
    }
}

/*eventmanager thread that sleeps for 20 milliseconds (or 20000 microseconds), checks headq1 (using a mutex for
 *  the two different ones so nothing can change it meanwhile)  which is a separate
 * list from the main one (each for the two events), if it is not null it unblocks the first thing in the list
 * and adds it back to the main list and notifies the consumer, same with headq2. If hey are both null then I
 * further check if the head of the main list is null. But this is not enough since sometimes for the last 
 * processes, the check might happens while consumer is moving it to a different list so i use an int called
 * timer and if for 10 iterrations through this thread, all heads are null every time, it then exits, else if one
 * of them gets to be not null, it sets it back to 10, this thread exits before consuemr thread
 */ 
void *eventQueue() {
    struct process *temp = NULL;
    int timer = 10;

    while(1){

       usleep(20000);
       
       pthread_mutex_lock(&q1);
       if (headq1 != NULL) {
           timer = 10;
           pthread_mutex_lock(&p);
           printf("Process %d in event queue 1 unblocked\n", headq1->iProcessId);
           pthread_mutex_unlock(&p);
           temp = getprocess(&headq1);
           add(&head, temp, &tail);
           pthread_mutex_unlock(&q1);
           sem_post(&full);
       }
       pthread_mutex_unlock(&q1);

       pthread_mutex_lock(&q2);
        if (headq2 != NULL) {
            timer = 10;
            pthread_mutex_lock(&p);
            printf("Process %d in event queue 2 unblocked\n", headq2->iProcessId);
            pthread_mutex_unlock(&p);
            temp = getprocess(&headq2);
            add(&head, temp, &tail);
            pthread_mutex_unlock(&q2);
            sem_post(&full);
        }
        pthread_mutex_unlock(&q2);
        if (headq1 == NULL && headq2 == NULL){
            pthread_mutex_lock(&lock);
            if (head == NULL && timer-- == 0) {
                pthread_mutex_unlock(&lock);
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&lock);
        }
    }
}

int main(void) {

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_mutex_init(&lock,NULL);
    pthread_mutex_init(&q1, NULL);
    pthread_mutex_init(&q2, NULL);
    pthread_mutex_init(&p, NULL);

    pthread_t producer;
    pthread_t c[NUMBER_OF_CONSUMERS];
    pthread_t eventManager;

    pthread_create(&producer, NULL, threadproduce, NULL);

    int consumerid[NUMBER_OF_CONSUMERS];
    int i = 0;
    for (i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        consumerid[i] = i;
        pthread_create(&(c[i]), NULL, threadconsume, (void *) &consumerid[i]);
    }
    
    pthread_create(&eventManager, NULL, eventQueue, NULL);

    pthread_join(producer, NULL);
    pthread_join(eventManager, NULL);

    sem_post(&full);   
    
    int n = 0;
    for(n = 0; n < NUMBER_OF_CONSUMERS; n++) {
        pthread_join(c[n], NULL);
    }

    printf("Average response time = %f \nAverage turnaround time = %f\n",
            (1.0* avgresponse/NUMBER_OF_PROCESSES), (1.0* avgturnaround/NUMBER_OF_PROCESSES));
    exit(EXIT_SUCCESS);
}
