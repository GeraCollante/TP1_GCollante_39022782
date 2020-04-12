#include "socket.h"
#include "mq.h"
#include "common.h"
#include <sys/types.h>
#include <sys/wait.h>

// struct sockaddr_in servaddr;
// long mtype = 10;

void    login_handler(int , int);
int     waitCli(int );

long get_1st_char(char * str){
    long m_type;
    char * aux  = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    snprintf(aux,2,"%s", str);
    m_type = atol(aux);
    return m_type;
}

long cmd_handler(char * cmd, char * msg){
    int process;
    long m_type;

    char * tok  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * prc  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * arg  = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    tok = strtok(cmd,",");
    sprintf(prc,"%s", tok);
    while (tok != NULL)
    {
        // printf ("%s\n",tok);
        sprintf(arg,"%s", tok);
        tok = strtok (NULL, ",");
    }
   
    // printf("m_type: %ld\n", get_1st_char(prc));
    m_type = get_1st_char(prc);
    process = atoi(prc);
    // printf("process: %d\n", process);
    // printf("arg: %s\n", arg);
    switch (process)
    {
    case 0:
        printf("exit\n");
        sprintf(msg, "%d", 0);
        break;
    case 10:
        printf("user ls\n");
        sprintf(msg, "%d", 1);
        break;
    case 11:
        printf("user passwd\n");
        sprintf(msg, "%d,%s", 2, arg);
        break;
    case 20:
        printf("file ls\n");
        sprintf(msg, "%d", 1);
        break;
    case 21:
        printf("file down\n");
        sprintf(msg, "%d,%s", 2, arg);
        break;
    default:
        break;
    }
    
    return m_type;
}

/**
 * @brief   Login handler between auth and server
 * @param   sockfd 
 * @param   msqid 
 */
void rcv_cmd(int sockfd, int msqid) 
{   
    char *  str = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    // char *  msg = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char    buff[MAX];
    long    m_type;
    int     end=1;
    do{ 
        /*  Clean buff  */
        memset(buff,0, MAX);
        /*  [SRV] <- [CLI]  */ 
        recv(sockfd, buff, sizeof(buff),0);
        printf("[SRV]<-[CLI]: %s\n", buff);
        fflush(stdout);
        m_type = cmd_handler(buff, str);
        if (m_type==0)  m_type = auth_type;
        printf("m_type: %ld\n", m_type);
        /*  [AUT] <- [SRV]  */
        puts("presnd");
        snd_msg(msqid, str, m_type);
        puts("postsnd");
        /*  [AUT] -> [SRV]  */
        rcv_msg(msqid, str, m_type);
        printf("-%s-", str);
        printf("atoi: %d\n", atoi(str));
        if(!strcmp(str,"Closed session."))
        {
            puts("SESION CERRADA");
            end = 0;
        }
        printf("[AUT]->[SRV]: -%s-\n",str);
        /*  [SRV] -> [CLI]  */
        sprintf(buff,"%s", str);
        send(sockfd, buff, sizeof(buff),0);
        printf("[SRV]->[CLI]: %s\n", buff);
    }while(end);

    printf("salimos del while");
    close(sockfd);
}

// Driver function 
int main() 
{ 
    pid_t child_a = fork();

    if (child_a == 0) 
    {
        /* Child A code */
        printf("auth_pid: %d \n", getpid());
        execv("./bin/auth", (char *[]){ NULL });
    } 
    else 
    {
        pid_t child_b = fork();
        if (child_b == 0) 
        {
            /* Child B code */
            printf("file_pid: %d \n", getpid());
            execv("./bin/fileserv", (char *[]){ NULL });
        } 
        else 
        {
            /* Parent Code */
            printf("srv_pid: %d \n", getpid());
            int sockfd, connfd, mqsid;
            // Create, bind and listing server socket
            mqsid  = mqid();
            printf("mqsid: %d\n", mqsid);
            mq_info(mqsid);
            sockfd = srv_socket();
            connfd = waitCli(sockfd);
            close(sockfd); 
            if (LOGIN)
            {
                login_handler(connfd, mqsid);
            }
            rcv_cmd(connfd, mqsid);

            // if (msgctl(mqsid, IPC_RMID, NULL) == -1) {
            //     fprintf(stderr, "Message queue could not be deleted.\n");
            //     exit(EXIT_FAILURE);
            // }
            printf("Finalice server. \n");

            wait(NULL);
        }
    }

    return 0;
} 

/**
 * @brief   Login handler between auth and server
 * @param   sockfd 
 * @param   msqid 
 */
void login_handler(int sockfd, int msqid) 
{   
    char * str = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char buff[MAX];
    int status;
    do{ 
        /*  Clean buff  */
        memset(buff,0, MAX); 
        /*  Read the message from client
            and copy it in buffer */ 
        recv(sockfd, buff, sizeof(buff),0); 
        /*  Print buffer which
            contains the client contents */
        printf("[SRV]<-[CLI]: %s\n", buff); 
        fflush(stdout);
        /*  Send client userpass
            to auth for authentication */
        snd_msg(msqid, buff, auth_type);
        /*  Receive status of  authentication */
        rcv_msg(msqid, str, auth_type);
        /*  Copy it to buffer and cast to int */ 
        sprintf(buff,"%s", str);
        //  printf("[AUT]->[SRV]: %s\n",buff);
        status = atoi(buff);
        /*  Send status to client   */
        printf("[SRV]->[CLI]: %s\n",buff);
        send(sockfd, buff, sizeof(buff),0); 
        /*  Exit from loop if login is succesful */
        if(status==1){ 
            break; 
        }
    }while(1);
    printf("[SRV] Successful login...\n"); 
    // close(sockfd);
}  


/**
 * @brief   Waiting for the client to connect
 * @param   sockfd 
 * @return  int sockfd
 */
int waitCli(int sockfd){
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
    return connfd;
}
