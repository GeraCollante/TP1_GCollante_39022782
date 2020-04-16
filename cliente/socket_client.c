#include "socket.h"
#include "common.h"

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
void recv_file(int newsockfd, int fd){
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
}


/**
 * @brief 	File is created and proceeds to receive
 * 			while the transfer time is calculated
 * @param 	newsockfd
 */
void transfer_file(int newsockfd){
	/* 	Declaration of variables 	*/
	int fd;
	char * filename = (char*) malloc((BUFFSIZE+1)*sizeof(char));

	/*	Set name to file	*/
	sprintf(filename, "%s", "img2burn");
	if(DEBUG)	printf("Receiving the image: %s\n",filename);
	
	/*	Get file descriptor	*/
	if ((fd=open(filename, O_CREAT|O_WRONLY,0600))<0){
		perror("Error creating file.\n");
		exit(EXIT_FAILURE);
	}
	
	/* 	Start clock		*/
	clock_t begin = clock();	
	/*	Receive and save file	*/
	recv_file(newsockfd, fd);
	/*	Stop clock	*/
	clock_t end = clock();

	/* 	File transfer time	*/
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	if(DEBUG)	printf("File transfer time: %g sec\n", time_spent);
}