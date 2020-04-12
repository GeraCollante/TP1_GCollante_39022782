#include "socket.h"

/**
 * @brief   Create, assign port, bind and listen of socket server
 * @return  int sockfd
 */
int srv_socket(){
    int sockfd;
    struct sockaddr_in servaddr;
  
    /*  Socket create and verification */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) { 
        perror("[SRV] Socket creation failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("[SRV] Socket successfully created..\n");
    
    /*  The socket can enter a TIME_WAIT state
        (to ensure all data has been transmitted, 
        TCP guarantees delivery if possible) 
        and take up to 4 minutes to release. 
        To avoid this time and speed up the test times, 
        SO_REUSEADDR is set that allows the socket
        to be available for immediate use.  */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    
    memset(&servaddr, 0, sizeof(servaddr));
    
    /*  Assign IP, PORT */
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY;  
    servaddr.sin_port = htons(PORT); 

    /*  Binding newly created socket to given IP and verification */
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        perror("[SRV] Socket bind failed\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("[SRV] Socket successfully binded..\n");
  
    /*  Now server is ready to listen and verification  */
    if ((listen(sockfd, 5)) != 0) { 
        perror("[SRV] Listen failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("[SRV] Server listening...\n"); 
    
    return sockfd;
}