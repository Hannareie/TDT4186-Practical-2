#include "bbuffer.h"
#include "sem.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

typedef struct BNDBUF {
    SEM *lowerLimit;
    SEM *upperLimit;

    long long* buffer;
    size_t size;
    size_t head;
    size_t tail;
} BNDBUF;

BNDBUF *bb_init(unsigned int size) {
    BNDBUF *bbuffer = malloc(sizeof(BNDBUF));
    bbuffer->lowerLimit = sem_init(0);
    bbuffer->upperLimit = sem_init(size);
    bbuffer->head = 0;
    bbuffer->tail = 0;
    bbuffer->size = size;
    bbuffer->buffer = malloc(size*sizeof(long long));

    return bbuffer;
}

void bb_del(BNDBUF *bb) {
    free(bb);
}

int bb_get(BNDBUF *bb) {
    P(bb->lowerLimit);
    V(bb->upperLimit);
    long long target = *(bb->buffer + bb->tail);
    (bb->tail) = (bb->tail + 1) % bb->size;
    return target;
}

void bb_add(BNDBUF *bb, int fd) {
    P(bb->upperLimit);
    V(bb->lowerLimit);
    bb->buffer[bb->head] = fd;
    bb->head = (bb->head + 1) % bb->size;
}