#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX     1024
#define PORT    10035
#define SA struct sockaddr 

int srv_socket();
int cli_socket();