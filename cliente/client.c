#include "client.h"
#include "md5.h"
#include "mbr.h"

#include <signal.h>

int sockfd;

void print_image(char * filename)
{
    FILE *fptr = NULL;
 
    if((fptr = fopen(filename,"r")) == NULL)
    {
        perror("Error opening");
        exit(EXIT_FAILURE);
    }
 
    char read_string[BUFFSIZE];
    while(fgets(read_string,sizeof(read_string),fptr) != NULL)
        printf("%s",read_string);
}

int main(){ 
    /*  Declaration of variables    */
    int counter = 0;
    char * str = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    int end;
    
    print_image("client.txt");
    /*  Create and connect client socket    */
    sockfd = cli_socket(PORT_SRV);
    
    /*  Handler for login   */
    if(LOGIN)   login_handler(sockfd);

    /*  Command handler */
    do{
        /*  Prompt for the user, 
            the function returns only valid commands */
        cmd_prompt(str);
        // if(DEBUG)   printf("Comando: -%s-\n", str);
        
        /*  Send the command to the server */
        end = cmd_handler(sockfd, str);
        if(DEBUG2)  printf("end: %d\n", end);

        /*  Counter of messages */
        counter ++;
        if(DEBUG2)  printf("message counter: %d\n", counter);
    } while(end!=1);

    /*  close the socket    */
    printf("Clossed session.\n");
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

    // if(DEBUG)   printf("nflags %d\n",nflags.c_lflag);

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
        if(DEBUG2)   printf("[SRV]<-[CLI]: %s\n", buff);
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
        if(DEBUG2)   printf("[SRV]->[CLI]: %s\n", buff);

        /*  Convert socket message to int for decoding  */
        status = atoi(buff);
        // if(DEBUG)   printf("status: %d\n", status);
        
        /*  According to the status obtained from the server,
            it is decided what to print and what actions to take    */
        switch(status)
        {
        case 1:
            printf("%s", "Successful ingress!\n");
            break;
        case 0:
            printf("%s", "Wrong password. Retry again.\n");
            break;
        case -1:
            printf("%s", "Non-existent user.\n");
            break;
        case -2:
            printf("%s", "User blocked.\n");
            break;
        }
    }while(check_status(status));

    if(DEBUG2)   printf("%s\n", "Salimos de login");
}

/**
 * @brief   If the image exists in the fileserver,
 *          it receives, saves, burns, erases and
 *          calculates the md5 and MBR of the USB.
 */
void file_down(){
    /*  Var declaration */
    char * usb = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    // char * img = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * md5 = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    long int bytes;
	time_t start,end;
  	double dif;

	/* 	Start clock		*/
  	time (&start);

    /*  Path */
    sprintf(usb, "%s", "/dev/sdc");
    // sprintf(img, "%s", "img2burn");

    /*  Create socket and connect to fileserver  */
    int socket = cli_socket(PORT_FLS);

    /*  Receive file    */
    bytes = transfer_file(socket, usb);
    printf("The file was successfully burned.\n");
    
    /*	Stop clock	*/
  	time (&end);
  	dif = difftime (end,start);
  	printf ("Image burn time was %.2lf seconds.\n", dif);

    /*  MD5     */
    get_md5(usb, bytes, md5);
    printf("MD5 checksum: %s \n", md5);

    mbr_table(usb);

}

/**
 * @brief   Send message to the server through a socket
 * @param   sockfd 
 * @param   cmd 
 */
int cmd_handler(int sockfd, char * cmd){
    /*  Variables declaration   */
    char buff[MAX];
    int end = 0;
    // char * buff = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    /*  Copy cmd to buff    */
    sprintf(buff, "%s", cmd);

    /*  Send socket message from client to server */
    if(DEBUG2)   printf("[SRV]<-[CLI]: %s\n", cmd);
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

    if(DEBUG2)   printf("[SRV]->[CLI]: %s\n", buff);

    switch(atoi(buff))
    {
    case -1:
        printf("File doesn't exists.\n");
        break;
    case 1:
        file_down();
        break;
    default:
        if (!strcmp(buff,"0"))
        {
            end = 1;
        }
        printf("%s\n", buff);
        break;
    }
    return end;
}

/**
 * @brief   Send a character 0 to the server that indicates an exit
 */
void ctrl_c(){
    /*  Variables declaration   */
    char buff[BUFFSIZE];

    /*  Copy cmd to buff    */
    sprintf(buff, "%s", "0");

    /*  Send socket message from client to server */
    if(send(sockfd, buff, sizeof(buff),0)<0){
        perror("Error in sending message to the [SRV].");
        exit(EXIT_FAILURE);
    };

    exit(EXIT_SUCCESS);
}

/**
 * @brief Signal handler
 */
void sigint_handler()
{
    printf("CTRL-C pressed\n");
    ctrl_c();
    exit(EXIT_SUCCESS);
}

/**
 * @brief   Create the signal and its structure, 
 *          we had to put return 0 if not give an error
 * @return  void* 
 */
void signal_init() {
  struct sigaction *sig = malloc(sizeof(struct sigaction));
  sig->sa_handler = &sigint_handler;
  sigaction(SIGINT, sig, NULL);
  return 0;
}