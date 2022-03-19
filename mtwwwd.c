#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXREQ (4096*1024)

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
void error(const char *msg) {
    perror(msg); 
    exit(1);
}

int main(int argc, char *argv[]) {
    char www_path[MAXREQ];
    char www_path_head[1024];
    int port;
    int num_threads;
    int num_buffer_slots;
    FILE *file;
    char *file_data;
    int file_size;

    //Vil kjøre programmet på formatet: mtwwwd www-path port #threads #bufferslots
    //www-path er (for meg) /home/hannareie/Oving_2_OPSYS/ 
    //threads og bufferslots kan velges til random nummer
    
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
    
    int server_socket, client_socket, data;
    socklen_t client_len;
    struct sockaddr_in server_address, client_address;
    
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
    
    while(1) {
        printf("Waiting for connections... \n");
        client_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_len);
        
        if (client_socket < 0) {
            error("Accept failed\n");
        } 
        printf("Connection accepted \n");

        bzero(buffer, sizeof(buffer));

        data = read(client_socket, buffer, sizeof(buffer) - 1); 
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
            "Content-Length: %d\n\n%s",
            strlen(body), body);
        
        data = write(client_socket, msg, strlen(msg));
        if (data < 0) error("Error writing to socket");
        close(client_socket);
    }
}