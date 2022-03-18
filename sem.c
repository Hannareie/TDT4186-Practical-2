#include "sem.h"
#include <pthread.h>
#include <errno.h>

typedef struct SEM {
    volatile int val;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SEM;

SEM *sem_init(int initVal) {
    SEM *sem = malloc(sizeof(SEM));

    if(!sem)
        goto Error1;

    sem->val = initVal;

    errno = pthread_mutex_init(&sem->mutex, NULL);
    if(!errno) 
        goto Error2;
    
    errno = pthread_cond_init(&sem->cond, NULL);
    if(!errno)
        goto Error3;
    
    return sem;

Error3:
    sem_del(sem); 
Error2:
    free(buf); // usikker på hva buf egt er, men vil tro vi skal sette inn vår bbuffer?
Error1: 
    return NULL 
}

void sem_del(SEM *sem) {
    //this method retunr 0 on sucess, else it returns an error number 
    pthread_mutex_destroy(&sem->mutex);
    free(buf); // håper dette løser problemet med å befri tilhørende ressurser
}

void P(SEM *sem) {
    pthread_mutex_lock(&sem->mutex);

    //Waiting for semaphore to have a positive value
    while(sem->val < 1)
        pthread_cond_wait(&sem->cond, &sem->mutex);
    
    --sem->val;

    //If any thread is waiting, wake it up
    if(sem->val > 0) 
        pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex); 
}

void V(SEM *sem) {
    pthread_mutex_lock(&sem->mutex);
    ++sem->val;

    //If any thread is waiting, wake it up
    if(sem->val > 0) 
        pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex); 
}