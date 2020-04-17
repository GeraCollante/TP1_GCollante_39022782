#include "socket.h"
#include "common.h"

/**
 * @brief   Waiting for the client to connect
 * @param   sockfd 
 * @return  int sockfd
 */
int wait_cli(int sockfd){
    /*  Variables */
    struct sockaddr_in cli;
    unsigned int len;
    int connfd;
    len = sizeof(cli);

    /*  Accept the data packet from client and verification */
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        perror("[SRV] Server accept failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("[SRV] Server accept the client...\n"); 
    close(sockfd);
    return connfd;
}

/**
 * @brief   Create, assign port, bind and listen of socket server
 * @return  int sockfd
 */
int srv_socket(int port){
    int sockfd;
    struct sockaddr_in servaddr;
  
    /*  Socket create and verification */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { 
        perror("Socket creation failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("Socket successfully created!\n");
    
    /*  The socket can enter a TIME_WAIT state
        (to ensure all data has been transmitted, 
        TCP guarantees delivery if possible) 
        and take up to 4 minutes to release. 
        To avoid this time and speed up the test times, 
        SO_REUSEADDR is set that allows the socket
        to be available for immediate use.  */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed.");
    
    memset(&servaddr, 0, sizeof(servaddr));
    
    /*  Assign IP, PORT */
    servaddr.sin_family         = AF_INET; 
    servaddr.sin_addr.s_addr    = INADDR_ANY;  
    servaddr.sin_port           = htons(port); 

    /*  Binding newly created socket to given IP and verification */
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) < 0) { 
        perror("Socket bind failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("Socket successfully binded!\n");
  
    /*  Now server is ready to listen and verification  */
    if ((listen(sockfd, 5)) < 0) { 
        perror("Listen failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
        printf("Server listening!\n"); 
    
    return sockfd;
}

/**
 * @brief   Send the file through sendfile function that 
 *          maximizes the transfer speed using zero copy
 * @param   connfd  socket file descriptor
 * @param   str     file without folder path
 */
void send_file(int connfd, char * str){
    /*  Variable declaration    */
    char * folder = "img";
    char * filename2 = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    struct stat file_stat;  //TODO malloc

    /*  Set path to file and get fd   */
    sprintf(filename2, "%s/%s", folder, str);
    int filefd = open(filename2, O_RDONLY);

    if(filefd < 0){
        perror("Can't open file.\n");
        exit(EXIT_FAILURE);
    };

    /*  Get file stats */
    fstat(filefd, &file_stat);

    /*  Send file   */
    if(sendfile(connfd, filefd, 0, (unsigned long) file_stat.st_size)<0){
        perror("Error sending file.\n");
        exit(EXIT_FAILURE);
    };

    /*  Send EOF string to warn recv in the client that it should
        not wait any longer because all the data has already been sent.
        Close fd.  */
    sprintf(str, "%s", "EOF");
    send(connfd, str, strlen(str), 0);
    close(filefd);
}