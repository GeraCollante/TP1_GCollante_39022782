#include "socket.h"

/**
 * @brief   Creation, assign IP and port of client socket.
 * Connect to server is included.
 * @return  sockfd 
 */
int cli_socket(){
    int sockfd; 
    struct sockaddr_in cliaddr; 
  
    /*  Socket create and verification */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        perror("[CLI] Socket creation failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("[CLI] Socket successfully created..\n"); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
  
    /*  Assign IP, PORT */ 
    cliaddr.sin_family = AF_INET; 
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    cliaddr.sin_port = htons(PORT); 
  
    /*  Connect the client socket to server socket */
    if (connect(sockfd, (SA*)&cliaddr, sizeof(cliaddr)) != 0) { 
        printf("[CLI] Connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("[CLI] Connected to the server..\n");
    return sockfd;
}