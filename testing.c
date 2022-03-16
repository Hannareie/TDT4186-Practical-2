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

#define SERVERPORT 6789
#define BUFSIZE (4096*1024)

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

int check(int exp, const char *msg);

int main(int argc, char **argv)
{
    int server_socket, client_socket;
    SA_IN server_addr, client_addr;
    char buffer[BUFSIZE], body[BUFSIZE], msg[BUFSIZE];
    socklen_t client_len;
    int n;

    check((server_socket = socket(PF_INET , SOCK_STREAM , 0)), "Failed to create socket");

    //Initialize the address struct
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);

    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)), "Bind failed");
    check(listen(server_socket, 10), "Listen failed");

    while (true) {

        client_len = sizeof(client_addr);
        check(client_socket = accept(server_socket, (SA*)&client_addr, &client_len), "Accept error");
        bzero(buffer, sizeof(buffer));
        check(n = read(client_socket, buffer, sizeof(buffer)-1), "ERROR reading from socket");
        snprintf(body, sizeof(body), "HTTP/1.1 200 OK"); 
        check(n = write(client_socket, msg, strlen(msg)), "ERROR writing to socket");
        close (client_socket);
    }
}

int check(int exp, const char *msg) {
    if (exp < 0) {
        perror(msg);
        exit(1);
    }
    return exp;
}