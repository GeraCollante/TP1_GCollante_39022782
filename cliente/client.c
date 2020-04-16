#include "client.h"
#include "md5.h"

int main(){ 
    /*  Declaration of variables    */
    int sockfd, contador = 0;
    char * str = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    
    /*  Create and connect client socket    */
    sockfd = cli_socket(PORT_SRV);
    
    /*  Handler for login   */
    if(LOGIN)   login_handler(sockfd);

    /*  Command handler */
    do{
        /*  Prompt for the user, 
            the function returns only valid commands */
        cmd_prompt(str);
        if(DEBUG)   printf("Comando: -%s-\n", str);
        
        /*  Send the command to the server */
        send_cmd(sockfd, str);

        /*  Counter of messages */
        contador ++;
        if(DEBUG)   printf("Contador de mensajes: %d\n", contador);
    } while(1);

    /*  close the socket    */
    close(sockfd); 
    return 0;
} 

/**
 * @brief   Prompt for userpass ingress
 * @param   userpass 
 * @return  userpass
 */
char * login(char * userpass){
    struct termios oflags, nflags;
    char    user[BUFFSIZE];
    char    pass[BUFFSIZE];

    /*  Get user */
    printf("Ingress user: ");
    if( fgets (user, sizeof(user), stdin)!=NULL ){
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

    if(DEBUG)   printf("nflags %d\n",nflags.c_lflag);

    /*  Set new terminal */
    if (tcsetattr(fileno(stdin), TCSADRAIN, &nflags) != 0){
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    /*  Get userpass */
    printf("Ingress pass: ");
    if( fgets (pass, sizeof(pass), stdin)!=NULL ){
        pass[strlen(pass) - 1] = 0;
    }
    sprintf(userpass, "%s,%s", user, pass);
    //Created userpass
    // printf("Hash is: %u",string_hash(user));
    // login = string_hash(user) == HASH ? 1 : 0;
    // login == 1 ? printf("Contraseña correcta") : printf("Contraseña incorrecta");

    /*  Active old terminal */
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0){ 
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    return userpass;
}

/**
 * @brief   Check status
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
    /*  Buffer for sockets  */
    char buff[MAX];
    char * userpass = (char*) malloc((BUFFSIZE*2+1)*sizeof(char));

    do
    {
        /*  Get userpass from prompt    */
        login(userpass);
        sprintf(buff, "%s", userpass);

        /*  Send socket message from client to server   */
        if(DEBUG)   printf("[SRV]<-[CLI]: %s\n", buff);
        if(send(sockfd, buff, sizeof(buff),0)<0){
            perror("Error in sending message to the server [login].");
            exit(EXIT_FAILURE);
        };

        /*  Receive socket message from server  */
        memset(buff, 0, MAX);
        if(recv(sockfd, buff, sizeof(buff),0)<0){
            perror("Error in receiving message from server [login].");
            exit(EXIT_FAILURE);
        };
        if(DEBUG)   printf("[SRV]->[CLI]: %s\n", buff);

        /*  Convert socket message to int for decoding  */
        status = atoi(buff);
        if(DEBUG)   printf("status: %d\n", status);
        
        /*  According to the status obtained from the server,
            it is decided what to print and what actions to take    */
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

    if(DEBUG)   printf("%s\n", "Salimos de login");
}

/**
 * @brief   Send message to the server through a socket
 * @param   sockfd 
 * @param   cmd 
 */
void send_cmd(int sockfd, char * cmd){
    /*  Variables declaration   */
    char buff[MAX];
    char * usb = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * img = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * md5 = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    sprintf(usb, "%s", "/dev/sdf");
    sprintf(img, "%s", "img2burn");
    // char * buff = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    /*  Copy cmd to buff    */
    sprintf(buff, "%s", cmd);

    /*  Send socket message from client to server */
    if(DEBUG)   printf("[SRV]<-[CLI]: %s\n", cmd);
    if(send(sockfd, buff, sizeof(buff),0)<0){
        perror("Error in sending message to the server [cmd].");
        exit(EXIT_FAILURE);
    };

    /*  Receive socket message from server */
    memset(buff, 0, MAX);
    if(recv(sockfd, buff, sizeof(buff),0)<0){
        perror("Error in receiving message from server [cmd].");
        exit(EXIT_FAILURE);
    };

    if(DEBUG)   printf("[SRV]->[CLI]: %s\n", buff);

    if(atoi(buff)==1){
        int socket = cli_socket(PORT_FLS);
        transfer_file(socket);
        printf("The file was successfully received.\n");
        printf("Burning image on USB. Please wait and do not disconnect the device.\n");
        burn_usb(img, usb);
        if(remove(img)<0){
            perror("Can't delete img.\n");
            exit(EXIT_FAILURE);
        };
        printf("The image was successfully burned on the USB device!\n");
        sprintf(usb, "%s1", usb);
        printf("The image is in partition %s\n", usb);
        file_md5(usb, md5);
        printf("md5: %s \n", md5);
    }

}

void burn_usb(char * img, char * usb){

    int imgfd = open(img, O_RDONLY);
    if(imgfd<0){
        perror("Error getting image file descriptor");
        exit(EXIT_FAILURE);
    }

    int usbfd = open(usb, O_WRONLY);
    if(usbfd<0){
        perror("Error getting usb file descriptor");
        exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    fstat(imgfd, &file_stat);

    if(sendfile(usbfd, imgfd, 0, (unsigned long) file_stat.st_size)<0){
        perror("Error sending file.\n");
        exit(EXIT_FAILURE);
    };

    close(imgfd);
    close(usbfd);
}