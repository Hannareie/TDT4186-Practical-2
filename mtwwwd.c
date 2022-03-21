#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
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

void error(const char *msg) {
    perror(msg); 
    exit(1);
}

void* handle_connection() {
    int thread_socket;
    ssize_t n;
    char* body;
    char* buffer;
    char* msg;

    FILE *fp;

    while(1) {
        thread_socket = bb_get(bbuffer);
        body = malloc(MAXREQ);
        buffer = malloc(MAXREQ);
        msg = malloc(MAXREQ);
        
        bzero(buffer, MAXREQ);

        n = read(thread_socket, buffer, MAXREQ - 1); 
        if (n < 0) error("Reading from socket failed");

        strcpy(www_path, www_path_head);
        strtok(buffer, " ");
        strcat(www_path, strtok(NULL, " "));

        memset(body, '\0', MAXREQ);

        fp = fopen(www_path,"rb");
        printf("%s\n", www_path);

        if (fp != NULL) {
            fread(body, sizeof(char), MAXREQ, fp);
            if (ferror(fp) != 0 ) {
            fputs("Error reading file", stderr);
            }
            fclose(fp);
        }

        snprintf(msg, sizeof(msg),
                "HTTP/1.0 200 OK\n"
                "Content-Type: text/html\n"
                "Content-Length: %d\n\n%s",
                strlen(body), body);
            
        n = write(client_socket, msg, strlen(msg));
        if (n < 0) error("Error writing to socket");

        close(thread_socket);
        free(buffer);
        free(msg);
        free(body);
    }
    printf("\nGoodbye.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 5) {
        strcpy(www_path_head, argv[1]);
        port = atoi(argv[2]);
        num_threads = atoi(argv[3]);
        num_buffer_slots = atoi(argv[4]);
    } else {
        error("Invoke this command using mtwwwd [www-path] [port] [#threads] [#bufferslots]");
    }
    printf("Program is started at port %i with given www_path: %s \n", port, www_path);
    
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
    
    if (listen(server_socket, 5) != 0) {
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