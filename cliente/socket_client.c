#include "socket.h"
#include "common.h"
#include <unistd.h>

/**
 * @brief   Creation, assign IP and port of client socket.
 *          Connect to server is included.
 * @return  sockfd 
 */
int cli_socket(int port){
    int sockfd; 
    struct sockaddr_in cliaddr; 
  
    /*  Socket create and verification */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
    { 
        perror("[CLI] Socket creation failed...\n"); 
        exit(EXIT_FAILURE); 
    }
    else
        printf("[CLI] Socket successfully created..\n"); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
  
    /*  Assign IP, PORT */ 
    cliaddr.sin_family = AF_INET; 
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    cliaddr.sin_port = htons(port); 
  
    /*  Connect the client socket to server socket */
    if (connect(sockfd, (SA*)&cliaddr, sizeof(cliaddr)) != 0){ 
        printf("[CLI] Connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("[CLI] Connected to the server..\n");
    
    return sockfd;
}

/**
 * @brief  	The file is received from the fileserver through a socket
 * @param 	newsockfd 
 * @param 	fd 
 */
long int recv_file(int newsockfd, int fd){
	/*	Declaration of variables	*/
	long int n, m, count=0;
	char buffer[BUFFSIZE];

	/*	Clean buffer	*/
	memset(buffer,0,BUFFSIZE);
	n=recv(newsockfd, buffer, BUFFSIZE,0);

	/* 	Receive the file until we find the EOF string 	*/
	while(n){
		if(!strcmp((char *)&buffer, "EOF")){
			if(DEBUG)	printf("End of file.\n");
			break;
		};
		if(n<0){
			perror("Error receiving file.\n");
			exit(EXIT_FAILURE);
		}
		if((m=write(fd,buffer,(unsigned long)n))<0){
			perror("Error writing file.\n");
			exit(EXIT_FAILURE);
		}
		count=count+m;
		/*	Clean buffer	*/
		memset(buffer,0,BUFFSIZE);
		n=recv(newsockfd,buffer,BUFFSIZE,0);
	}

	/*	Close file descriptor of file and socket	*/
	close(fd);
	close(newsockfd);
	return count;
}

/**
 * @brief 	File is created and proceeds to receive
 * 			while the transfer time is calculated
 * @param 	newsockfd
 */
long int transfer_file(int newsockfd, char * usb){
	/* 	Declaration of variables 	*/
	int fd;
	long int bytes;
	// char * filename = (char*) malloc((BUFFSIZE+1)*sizeof(char));

	time_t start,end;
  	double dif;

	if(DEBUG)	printf("Receiving the image.\n");
	
	/*	Get file descriptor	*/
	if ((fd=open(usb, O_WRONLY))<0){
		perror("Error creating file.\n");
		exit(EXIT_FAILURE);
	}
	
	/* 	Start clock		*/
  	time (&start);

	/*	Receive and save file	*/
	bytes = recv_file(newsockfd, fd);

	/*	Stop clock	*/
  	time (&end);
  	dif = difftime (end,start);
  	printf ("Image burn time was %.2lf seconds.\n", dif);
	sync();

	return bytes;
}