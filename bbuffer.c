#include "bbuffer.h"
#include "sem.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>

typedef struct BNDBUF {
    unsigned int size;
    SEM lowerLimit;
    SEM upperLimit;

    void *head;
    void *tail;

    struct BNDBUF next;
} BNDBUF;


BNDBUF *bb_init(unsigned int size) {
    SEM sem;
    BNDBUF *bndbuf = malloc(sizeof(BNDBUF));
    bndbuf->semLowerLimit = *sem_init(0);
    bndbuf->semUpperLimit = *sem_init(size);
    bndbuf->capacity = size;

    if (!bndbuf)
    {
        goto Error1;
    }

    bndbuf->size = size;

    return bndbuf;
Error1:
    return ((void *)0);
}


//size - The number of integers that can be stored in the bounded buffer.
//returns (head) handle for the created bounded buffer, or NULL if an error occured.

void bb_del(BNDBUF *bb) {
    free(bb -> semLowerLimit);
    free(bb -> semUpperLimit);
    free(bb -> capacity);
    free(bb -> size);
    free(bb -> head);
    free(bb -> tail);
}

//bb - Handle of the bounded buffer.

int  bb_get(BNDBUF *bb) {
    if (head == NULL) {
        return NULL;
    } else{
        int *result = head->client_socket;
        BNDBUF *temp = head;
        head = head -> next;
        if (head == NULL) {tail = NULL;}
        free(temp);
        return result;
    }
}

//bb - Handle of the bounded buffer.

void bb_add(BNDBUF *bb, int fd) {
    BNDBUF *newNode = malloc(sizeof(buff_node));
    newNode -> client_socket = client_socket;
    newNode -> next = NULL;
    if (tail == NULL) {
        head = newnode;
    } else {
        tail -> next = newNode;
    }
    tail = newNode;
}

//bb - (head) Handle of the bounded buffer.
//fd -  Value that shall be added to the buffer.
