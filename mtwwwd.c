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

int server_socket, client_socket;
socklen_t client_len;
struct sockaddr_in6 server_address, client_address;

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
        //Get a socket from the buffer
        thread_socket = bb_get(bbuffer);
        body = malloc(MAXREQ);
        buffer = malloc(MAXREQ);
        msg = malloc(MAXREQ);
        
        //Resetting the buffer
        bzero(buffer, MAXREQ);

        //Reading the request from the socket to buffer
        n = read(thread_socket, buffer, MAXREQ-1); 
        if (n < 0) error("Reading from socket failed");
        memset(body, '\0', MAXREQ);

        //File formatting
        strcpy(www_path, www_path_head);
        strtok(buffer, " ");
        strcat(www_path, strtok(NULL, " "));

        //Opening the given path
        fp = fopen(www_path,"r");

        if (!fp) {
            snprintf(body, MAXREQ, 
            "%s",
            "<html><body><h1> 404 File Not Found </h1></body></html>\r\n");
        } else {
            //Reading from the file to body and closing the file
            fread(body, MAXREQ, 1, fp);
            fclose(fp);
        }
        
        //Generating the response
        snprintf(msg, MAXREQ,
                "HTTP/1.0 200 OK\n"
                "Content-Type: text/html\n"
                "Content-Length: %d\n\n%s\n", MAXREQ, body);

        //Sending the response to the socket
        n = write(thread_socket, msg, strlen(msg));
        if (n < 0) {
            error("Error writing to socket");
        };

        //Closing the connection
        close(thread_socket);
        free(buffer);
        free(msg);
        free(body);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    //Handling the command arrguments on format: www-path port #threads #bufferslots
    if (argc == 5) {
        strcpy(www_path_head, argv[1]);
        port = atoi(argv[2]);
        num_threads = atoi(argv[3]);
        num_buffer_slots = atoi(argv[4]);
    } else {
        error("Error: The format should be mtwwwd www-path port #threads #bufferslots");
    }
    printf("Program is started at port %i with given www_path: %s\n", port, www_path_head);
    
    //Creating a new socket that allows both IPv4 and IPv6
    server_socket = socket(PF_INET6, SOCK_STREAM, 0);

    if (server_socket == -1) {
        error("Error opening socket\n");
    }
    printf("Socket successfull created\n");
    
    //Binging the socket to a given address
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port);
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("Error while binding socket\n");
    }
    
    //Listening to the socket
    if (listen(server_socket, 5) != 0) {
        error("Error occured while listening to socket\n");
    }
    printf("Listening...\n");

    // Creating the buffer
    bbuffer = bb_init(num_buffer_slots);

    // Creating the thread
    int thread;
    pthread_t server_threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        thread = pthread_create(&server_threads[i], NULL, handle_connection, NULL);
    }

    while(1) {
        printf("Waiting for connections...\n");
        //Set length of client address
        client_len = sizeof(client_address);

        //Accepting a new connection
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_len);
        
        if (client_socket < 0) {
            error("Accept failed\n");
        } 
        printf("Connection accepted\n");

        //Adding the new socket to the buffer
        bb_add(bbuffer, client_socket);
    }
    bb_del(bbuffer);
    return 0;
}