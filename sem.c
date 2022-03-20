#include "sem.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct SEM {
    volatile int val;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SEM;

SEM *sem_init(int initVal) {
    SEM* sem = malloc(sizeof(struct SEM));
    sem->val = initVal;

    if (pthread_mutex_init(&sem->mutex, NULL)) {
        sem_del(sem);
        return NULL;
    }

    if (pthread_cond_init(&sem->cond, NULL)) {
        sem_del(sem);
        return NULL;
    }
    return sem;
}

int sem_del(SEM *sem) {
    int result = 0;
    if (pthread_mutex_destroy(&sem->mutex) > 0) {
        result -= 1;
    }
    if (pthread_cond_destroy(&sem->cond) > 0) {
        result -= 2;
    }
    free(sem);
    return result;
}

void P(SEM *sem) {
    pthread_mutex_lock(&sem->mutex);
    while (sem->val < 1) {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    
    sem->val--;
    pthread_mutex_unlock(&sem->mutex); 
}

void V(SEM *sem) {
    pthread_mutex_lock(&sem->mutex);
    sem->val++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex); 
}

int get_value(SEM *sem) {
  return sem->val;
}