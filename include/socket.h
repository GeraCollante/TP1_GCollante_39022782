#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/sendfile.h>
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>
#include <time.h>

#define MAX         1024
#define SA struct   sockaddr

int         cli_socket      (int);
long int    recv_file       (int, int);
void        send_file       (int, char *);
int         srv_socket      (int);
long int    transfer_file   (int, char *);
int         wait_cli        (int);