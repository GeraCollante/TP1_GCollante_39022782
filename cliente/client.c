#include "socket.h"
#include "prompt.h"
#include "common.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>

/**
 * @brief   Prompt for userpass ingress
 * @param   userpass 
 * @return  userpass
 */
char * login(char * userpass){
    struct termios oflags, nflags;
    char    user[BUFFSIZE];
    char    pass[BUFFSIZE];

    printf("Ingress user: ");
    if( fgets (user, sizeof(user), stdin)!=NULL ) {
        user[strlen(user) - 1] = 0;
    }

    /*  Get old terminal settings   */
    tcgetattr(fileno(stdin), &oflags);
    /*  Save old terminal in new terminal */
    nflags = oflags;
    /*  Desactive echo flags    */
    // nflags.c_lflag &= ~ECHO;
    // nflags.c_lflag |= ECHONL;
    nflags.c_lflag = 35443;

    printf("nflags %d\n",nflags.c_lflag);

    /*  Set new terminal */
    if (tcsetattr(fileno(stdin), TCSADRAIN, &nflags) != 0) // Set new terminal
    {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    /*  Prompt for userpass */
    printf("Ingress pass: ");
    if( fgets (pass, sizeof(pass), stdin)!=NULL ) {
        pass[strlen(pass) - 1] = 0;
    }
    sprintf(userpass, "%s,%s", user, pass);
    //Created userpass
    // printf("Hash is: %u",string_hash(user));
    // login = string_hash(user) == HASH ? 1 : 0;
    // login == 1 ? printf("Contraseña correcta") : printf("Contraseña incorrecta");

    /*  Active old terminal */
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
    { 
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    // printf("Userpass: %s", userpass);
    return userpass;
}

/**
 * @brief   Check if status if 0
 * @param   status 
 * @return  status
 */
int check_status(int status){
    status = (status==1) ? 0 : 1;
    return status;
}

/**
 * @brief   The connection between the server and the client is handled from this function,
 *          which is also responsible for the message passage between these.
 * @param   sockfd
 */
void login_handler(int sockfd){
    int status;
    /*  Buffer for sockets */
    char buff[MAX];
    char * userpass = (char*) malloc((BUFFSIZE*2+1)*sizeof(char));

    do
    {
        /*  Get userpass from prompt */
        login(userpass);
        sprintf(buff, "%s", userpass);

        /*  Send socket message from client to server */
        printf("[SRV]<-[CLI]: %s\n", buff);
        send(sockfd, buff, sizeof(buff),0);
        memset(buff, 0, MAX);

        /*  Receive socket message from server */
        recv(sockfd, buff, sizeof(buff),0);
        printf("[SRV]->[CLI]: %s\n", buff);
        status = atoi(buff);
        // printf("status: %d\n", status);
        
        /*  According to the status obtained from the server,
            it is decided what to print and what actions to take */
        switch(status)
        {
        case 1:
            printf("%s", "Successful ingress!\n");
            break;
        case 0:
            printf("%s", "Wrong password\n");
            break;
        case -1:
            printf("%s", "Non-existent user\n");
            break;
        case -2:
            printf("%s", "User blocked\n");
            break;
        }
    }while(check_status(status));
    printf("%s\n", "Salimos de login");
}

void send_cmd(int sockfd, char * cmd){
    char buff[MAX];
    sprintf(buff, "%s", cmd);

    /*  Send socket message from client to server */
    printf("[SRV]<-[CLI]: %s\n", buff);
    send(sockfd, buff, sizeof(buff),0);

    /*  Receive socket message from server */
    memset(buff, 0, MAX);
    recv(sockfd, buff, sizeof(buff),0);
    printf("[SRV]->[CLI]: %s\n", buff);
}

int main() 
{ 
    int sockfd;    // Create and connect client socket
    sockfd = cli_socket();
    // function for chat 
    // func(sockfd);
    if(LOGIN){
        login_handler(sockfd);
    }
    char * str  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    // // Run command loop.
    int contador = 0;
    do
    {
        cmd_prompt(str);
        printf("Comando: -%s-\n", str);
        send_cmd(sockfd, str);
        contador ++;
        printf("Contador de mensajes: %d\n", contador);
    } while(1);

    // send_cmd(sockfd);
    // close the socket 
    close(sockfd); 
} 