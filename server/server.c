#include "server.h"

int main() 
{ 
    /*  Create child A  */
    pid_t child_a = fork();

    if (child_a == 0) 
    {
        /*  Child A code    */
        printf("[AUT]_pid: %d \n", getpid());
        execv("./bin/auth", (char *[]){ NULL });
    } 
    else 
    {
        pid_t child_b = fork();
        if (child_b == 0) 
        {
            /*  Child B code    */
            printf("[FLS]_pid: %d \n", getpid());
            execv("./bin/fileserv", (char *[]){ NULL });
        } 
        else 
        {
            /*  Parent Code     */
            printf("[SRV]_pid: %d \n", getpid());
            int sockfd, connfd, mqsid;

            /*  Create message queue */
            mqsid = mqid();
            // if(DEBUG)   mq_info(mqsid);
            if(DEBUG)   printf("[SRV] Created message queue...\n");

            /*  Bind and listing server socket  */
            sockfd = srv_socket(PORT_SRV);
            /*  Waiting for client  */
            connfd = wait_cli(sockfd);
            /*  Close initial fd    */
            close(sockfd); 

            /*  Login handler   */
            if (LOGIN)  login_handler(connfd, mqsid);

            /*  Cmd handler */
            rcv_cmd(connfd, mqsid);

            // if (msgctl(mqsid, IPC_RMID, NULL) == -1) {
            //     fprintf(stderr, "Message queue could not be deleted.\n");
            //     exit(EXIT_FAILURE);
            // }
            printf("End [SRV]. \n");

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
        /*  Read the message from [CLI]
            and copy it in buffer */ 
        memset(buff,0, MAX); 
        recv(sockfd, buff, sizeof(buff),0); 
        if(DEBUG)   printf("[SRV]<-[CLI]: %s\n", buff); 
        fflush(stdout);

        /*  Send userpass to [AUT] for authentication */
        snd_msg(msqid, buff, auth_type);
        
        /*  Receive status of [AUT] */
        rcv_msg(msqid, str, auth_type);
        /*  Copy it to buffer and cast to int */ 
        sprintf(buff,"%s", str);
        status = atoi(buff);

        /*  Send status to [CLI]   */
        printf("[SRV]->[CLI]: %s\n",buff);
        send(sockfd, buff, sizeof(buff),0); 

        //ARREGLAR
        /*  Exit from loop if login is succesful */
        if(status==1){ 
            break; 
        }
    }while(1);

    if(DEBUG)   printf("[SRV] Successful login...\n");
}  

/**
 * @brief   Get the 1st char of str   
 * @param   str 
 * @return  long 
 */
long get_1st_char(char * str){
    long m_type;
    char * aux  = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    snprintf(aux,2,"%s", str);
    m_type = atol(aux);
    return m_type;
}

/**
 * @brief   Decode the str that comes from the client and
 *          then be forwarded to AUT or FLS as appropriate
 * @param   cmd 
 * @param   msg 
 * @return  long m_type
 */
long cmd_handler(char * cmd, char * msg){
    int process;
    long m_type;

    char * tok  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * prc  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * arg  = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    tok = strtok(cmd,",");
    /*  Get process */
    sprintf(prc,"%s", tok);
    while (tok != NULL)
    {
        /*  Get the remaining arguments */
        sprintf(arg,"%s", tok);
        tok = strtok (NULL, ",");
    }
   
    /*  The first number in the string corresponds to the process,
        which in turn identifies the m_type */
    m_type  = get_1st_char(prc);
    /*  The two numbers represent the command that the process
        must execute, therefore the message that will be sent
        to AUT or FLS is decided based on this  */
    process = atoi(prc);
    switch (process)
    {
    case 0:
        if(DEBUG)   printf("exit\n");
        sprintf(msg, "%d", 0);
        break;
    case 10:
        if(DEBUG)   printf("user ls\n");
        sprintf(msg, "%d", 1);
        break;
    case 11:
        if(DEBUG)   printf("user passwd\n");
        sprintf(msg, "%d,%s", 2, arg);
        break;
    case 20:
        if(DEBUG)   printf("file ls\n");
        sprintf(msg, "%d", 1);
        break;
    case 21:
        if(DEBUG)   printf("file down\n");
        sprintf(msg, "%d,%s", 2, arg);
        break;
    default:
        break;
    }

    printf("msg: %s\n", msg);
    
    return m_type;
}

/**
 * @brief   Login handler between [AUT] and [SRV]
 * @param   sockfd 
 * @param   msqid 
 */
void rcv_cmd(int sockfd, int msqid) 
{   
    char *  str = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char    buff[MAX];
    long    m_type;
    int     end=1;
    do{ 
        /*  [SRV] <- [CLI]  */ 
        memset(buff,0, MAX);
        recv(sockfd, buff, sizeof(buff),0);
        printf("[SRV]<-[CLI]: %s\n", buff);
        fflush(stdout);

        /*  Pass cmd to the handler  */
        m_type = cmd_handler(buff, str);

        /*  This is exit command    */
        if(m_type==0)  m_type = auth_type;
        // if(DEBUG)   printf("m_type: %ld\n", m_type);

        /*  [AUT] <- [SRV]  */
        printf("[AUT]<-[SRV]: %s\n", str);
        snd_msg(msqid, str, m_type);
        
        /*  [AUT] -> [SRV]  */
        rcv_msg(msqid, str, m_type);

        /* If AUT sent this string it means that an exit
            was sent and the session will end   */
        if(!strcmp(str,"Closed session."))  end = 0;
        if(DEBUG)   printf("[AUT]->[SRV]: -%s-\n",str);
        
        /*  [SRV] -> [CLI]  */
        sprintf(buff,"%s", str);
        send(sockfd, buff, sizeof(buff),0);
        if(DEBUG)   printf("[SRV]->[CLI]: %s\n", buff);
    }while(end);

    close(sockfd);
}