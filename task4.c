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
//additional lock for when I am printing 
pthread_mutex_t p;

struct process *tail = NULL;
struct process *head = NULL;

int avgresponse = 0;
int avgturnaround = 0;

//function to add to the tail to the linked list
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

//function that uses temp to save the current head and then makes head to point to the next thing in list
struct process *getprocess(struct process **head) {
    struct process *temp = NULL;
    temp = *head;
    *head = (*head)->oNext;
    temp->oNext = NULL;
    return temp;
}

/*function to print result according to if it has run
for the first time (state = 1) , printing only response time or if it has finished, printing turn around time
*/
void print (int processid, int istate, int previous, int new, int state, int id, long int turnaround, long int response){
    printf("Consumer Id = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d", id, processid, previous, new);
    if (state == 1) {
        printf(", Response time = %ld", response);
    }
    if (istate == FINISHED){
        printf(", Turn Around time = %ld", turnaround);
    }
    printf("\n");    
}

/*producer thread that produces number of specified processes and wakes up consumer, it uses mutex to prevent 
 * something interrupting when accessing a global variable or preventing it from changing value meanwhile 
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
        a++;
        sem_post(&full);
    }
    pthread_exit(NULL);
}

/*consumer thread(takes a void pointer to the consumer id) that uses a while(1) loop , gets woken up by producer
 * checks if head is null and if it is
 * it then adds every consumer's own sum of the response and turnaround time to the global avgresponse/turnaround
 * time, tells the other threads that they can exit using a sem_post and then exits. If head is not null it then
 * gets a process, and assignes a state to it according to being new process or not, simulates round robin
 * and calculates response and turnaround time, adds the response when the process is new and turnaround time
 * when process is either new and finished with one iterration or if it has run more times and it finished.
 * if process hasn;t finished, it gets added back to the queue and tells consumer thread something is in there
 * and if it has finished then it frees the struct and wakes up producer to produce if buffer is less than 5
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
            simulateRoundRobinProcess(proc, &oTimeStart, &oTimeEnd);
            newburst = proc->iBurstTime;
            istate = proc->iState;
            turnaroundtime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeEnd);
            responsetime = getDifferenceInMilliSeconds(proc->oTimeCreated, oTimeStart);
            pthread_mutex_lock(&p);
            print(processid, istate, prevburst, newburst, state, consumer_id, turnaroundtime, responsetime);
            pthread_mutex_unlock(&p);

            if(state == 1) {
                sumresponse += responsetime;
            } else if(proc->iState == FINISHED) {
                sumturnaround += turnaroundtime;
            }

            if(proc->iState == READY) {
               pthread_mutex_lock(&lock);
               add(&head, proc, &tail);
               sem_post(&full);
               pthread_mutex_unlock(&lock);

            } else if(proc->iState == FINISHED) {
                sem_post(&empty);
                free(proc);
            }

    }
}

int main(void) {

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&p, NULL);

    pthread_t producer;
    pthread_t c[NUMBER_OF_CONSUMERS];

    pthread_create(&producer, NULL, threadproduce, NULL);

    int consumerid[NUMBER_OF_CONSUMERS];
    for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        consumerid[i] = i;
        pthread_create(&(c[i]), NULL, threadconsume, (void *) &consumerid[i]);
    }
    

    pthread_join(producer, NULL);
    sem_post(&full);   
    for(int n = 0; n < NUMBER_OF_CONSUMERS; n++) {
        pthread_join(c[n], NULL);
    }

    printf("Average response time = %f \nAverage turnaround time = %f\n",
            (1.0* avgresponse/NUMBER_OF_PROCESSES), (1.0* avgturnaround/NUMBER_OF_PROCESSES));
    exit(EXIT_SUCCESS);
}
