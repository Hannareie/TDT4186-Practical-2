#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "bbuffer.h"

#define MAXREQ (4096*1024)

char www_path[MAXREQ];
char www_path_head[1024];
int port;
int num_threads;
int num_buffer_slots;

BNDBUF* bbuffer;

int server_socket, client_socket, data;
socklen_t client_len;
struct sockaddr_in server_address, client_address;

//char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
void error(const char *msg) {
    perror(msg); 
    exit(1);
}

void* handle_connection() {
    long body_size = 0;
    int thread_socket;
    ssize_t data;
    char* body;
    char* buffer;
    char* msg;

    FILE *file;
    char *file_data;
    int file_size;
    
    while(1) {
        thread_socket = bb_get(bbuffer);
        body = malloc(MAXREQ);
        buffer = malloc(MAXREQ);
        msg = malloc(MAXREQ);
        
        bzero(buffer, sizeof(buffer));

        data = read(thread_socket, buffer, sizeof(buffer) - 1); 
        if (data < 0) error("Reading from socket failed");
        
        strcpy(www_path, www_path_head);
        strtok(buffer, " ");
        strcat(www_path, strtok(NULL, " "));
        
        file = fopen(www_path,"r");
        
        if (file != NULL) {
            size_t new_len = fread(body, sizeof(char), MAXREQ, file);
            if (ferror(file) != 0 ) {
                fputs("Reading from socket failed", stderr);
            } else {
                body[new_len++] = '\0'; 
            }
            fclose(file);
        }
        
        snprintf(msg, sizeof(msg), 
            "HTTP/1.0 200 OK\n"
            "Content-Type: text/html\n"
            "Content-Length: %ld\n\n%s",
            strlen(body), body);
        
        data = write(client_socket, msg, strlen(msg));
        if (data < 0) error("Error writing to socket");
        close(thread_socket);
        free(buffer);
        free(msg);
        free(body);
    }
    printf("\nGoodbye.\n");
    return 0;
}

int main(int argc, char *argv[]) {

    //Format: mtwwwd www-path port #threads #bufferslots
    //gcc -o mtwwwd mtwwwd.c
    //./mtwwwd /home/hannareie/Oving_2_OPSYS/docs 8989 5 5 
    
    if (argc > 1) {
        strcpy(www_path_head, argv[1]);
    } else {
        error("No www-path is given\n");
    } 
    if (argc > 2) {
        sscanf(argv[2], "%d", &port);
    } else {
        error("No port is given\n");
    }
    if (argc > 3) {
        sscanf(argv[3], "%d", &num_threads);
    } else {
        error("Number of threads is not given\n");
    }
    if (argc > 4) {
        sscanf(argv[4], "%d", &num_buffer_slots);
    } else {
        error("Number of buffer slots is not given\n");
    }
    
    printf("Program is started at port %i with given www_path: %s \n", port, www_path_head);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        error("Error opening socket\n");
    }
    printf("Socket successfull created\n");
    
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("Error while binding socket\n");
    }
    
    if (listen(server_socket, 10) != 0) {
        error("Error occured while listening to socket \n");
    }
    printf("Listening...\n");

    bbuffer = bb_init(num_buffer_slots);
    int thread;
    pthread_t server_threads[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread = pthread_create(&server_threads[i], NULL, handle_connection, NULL);
    }

    while(1) {
        printf("Waiting for connections... \n");
        client_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_len);
        
        if (client_socket < 0) {
            error("Accept failed\n");
        } 
        printf("Connection accepted \n");
        bb_add(bbuffer, client_socket);
    }
    bb_del(bbuffer);
    return 0;
}