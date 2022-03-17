/*
    Commands to run a server
    -> gcc -pthread -o server mtwwwd.c (server kan byttes til annet navn)
    -> ./server
    This generates an execute file named server. Remember to delete this file before pushing new code.
    
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SERVER_BACKLOG 100

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void * handle_connection(void* client_socket);
int check(int exp, const char *msg);

sem_t mutex;

int main(int argc, char **argv)
{
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    check((server_socket = socket(AF_INET , SOCK_STREAM , 0)), 
            "Failed to create socket");

    //Initialize the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);

    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)), 
            "Bind failed");
    check(listen(server_socket, SERVER_BACKLOG), 
            "Listen failed");
    printf("Listening...");

    while (true) {
        printf("Waiting for connections... \n");
        //Wait for and accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = 
                accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size), 
                "Accect failed");
        printf("Connected\n");

        sem_init(&mutex, 0, 1);
        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;

        if (pthread_create(&t, NULL, handle_connection, &client_socket) != 0)
            // Error in creating thread
            printf("Failed to create thread\n");
        sem_destroy(&mutex);
    }
    return 0;
}

int check(int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void * handle_connection(void* p_client_socket) {

    sem_wait(&mutex);
    printf("\nEntered..\n");

    int client_socket = *((int*)p_client_socket);
    //free(p_client_socket); 
    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX+1];

    while((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer)-msgsize-1))> 0) {
        msgsize += bytes_read;
        if (msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break;
    }

    check(bytes_read, "revc error");
    buffer[msgsize-1] = 0;

    printf("REQUEST: %s\n", buffer);
    fflush(stdout);

    if (realpath(buffer, actualpath) == NULL) {
        printf("Error(Bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL) {
        printf("ERROR(open): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    while((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        printf("sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    printf("\nJust Exiting...\n");
    sem_post(&mutex);

    close(client_socket);
    fclose(fp);
    printf("Closing connection\n");
    //pthread_exit(NULL);
    return NULL;
}