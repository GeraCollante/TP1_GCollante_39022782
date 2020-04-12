#include "auth.h"
#include "mq.h"
#include <sys/types.h>
#include <sys/wait.h>

void    change_pass(char *, char *);
int     check_pass(char *, char *);
char *  list_users(char *);
void    load_db(void);
char *  login_handler(int, char *);
void    save_db(void);

/**
 * @brief Variable that determines the id of the message queue corresponding to the process
 */
// long mtype = 10;

void last_connect(char* user){
    char * connect_time = (char*) malloc((BUFFSIZE*USERS+1)*sizeof(char));
    time_t rawtime;
    struct tm * timeinfo;

    /* Equals to current_time = time(NULL); */
    if(time(&rawtime)<0){
        perror("Error to get current time.\n");
        exit(EXIT_FAILURE);
    };

    /* Create a structure with different fields 
    like seconds, minutes, hours, etc. */
    timeinfo = localtime(&rawtime);
    sprintf(connect_time, "%d/%d/%d %d:%d:%d",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    for (int i = 0; i < USERS; i++)
    {
        if(!strcmp(user, users[i].username)){
            strcpy(users[i].lastconnect, connect_time);
            printf("Nueva lastconnect: %s\n", users[i].lastconnect);
            break;
        }
    }
    save_db();
}

/**
 * @brief Increase number of blocks according to user
 * @param user 
 */
void increase_block(char* user){
    int blocks;
    for (int i = 0; i < USERS; i++)
    {
        if(!strcmp(user, users[i].username)){
            /*  get blocks count,
                increase and update DB */
            blocks = atoi(users[i].block);
            blocks++;
            sprintf(users[i].block, "%d\n", blocks);
            if(DEBUG) printf("Cantidad de bloqueos: %s\n", users[i].block);
            break;
        }
    }
    save_db();
}

/**
 * @brief Check the number of user blocks
 * @param user
 * @return int
 * 0 if user is ok
 * 1 if user is block
 */
int check_block(char* user){
    int status;
    int blocks=0;
    for (int i = 0; i < USERS; i++)
    {
        if(!strcmp(user, users[i].username)){
            /*  User exists, 
                password will be checked */
            blocks = atoi(users[i].block);
            break;
        }
    }
    status = (blocks>2) ? 1 : 0;
    return status;
}

/**
 * @brief   Get the status of check userpass
 * @param   userpass 
 * @return  int 
 *          1:  user and pass correct 
 *          0:  pass wrong
 *          -1:  user wrong
 *          -2:  user blocked
 */
int get_status(char * userpass, char * user){
    int status;
    char * tok  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    // char * user = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * pass = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    // char str[] ="- This, a sample string.";
    // sprintf(str, "%s", "GeraCollante,steven95");
    // char * tok;
    // printf ("Splitting string \"%s\" into toks:\n",str);
    tok = strtok(userpass,",");
    sprintf(user,"%s", tok);
    while (tok != NULL)
    {
        // printf ("%s\n",tok);
        sprintf(pass,"%s", tok);
        tok = strtok (NULL, ",");
    }
    if(DEBUG) printf("user: %s \n", user);
    if(DEBUG) printf("pass: %s \n", pass);

    /*  Check if user is blocked if not check password */
    status = (check_block(user)) ? -2 : check_pass(user,pass);
    /*  If wrong password then increase blocks value */
    if(!status)     increase_block(user);
    if(DEBUG) printf("status: %d\n",status);
    return status;
}

/**
 * @brief   Login handler between auth and srv
 * @param   msqid 
*/
char * login_handler(int msqid, char * user){
    int status;
    char * str  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    do{
        /*  Receive userpass from [SVR] */
        rcv_msg(msqid, str, auth_type);
        printf("[AUT]<-[SRV]: %s\n",str);
        /*  Get status of userpass and convert to int */
        sprintf(str, "%d", get_status(str, user));
        status = atoi(str);
        printf("[AUT]->[SRV]: %s\n",str);
        /*  Send status to server */ 
        snd_msg(msqid, str, auth_type);
        if(status==1)
        { 
            if(DEBUG) printf("Successful login_handler...\n"); 
            break;
        }
    }while(1);
    return user;
}

char * get_pass(char * str)
{
    char * aux = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    strtok(str, ",");
    sprintf(aux, "%s", strtok(NULL, ","));
    return aux;
}

/**
 * @brief   Login handler between auth and srv
 * @param   msqid 
*/
int cmd_handler(int msqid, char * user){
    int status;
    // int salir = 0;
    char * str  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    
    /*  Receive userpass from [SVR] */
    rcv_msg(msqid, str, auth_type);
    printf("[AUT]<-[SRV]: %s\n",str);
    fflush(stdout);

    /*  Get status of userpass and convert to int */
    status = atoi(str);
    // printf("predecoder\n");
    // cmd_decoder(status, str);
    // printf("postdecoder\n");
    
    /*  Decode cmd  */
    switch (status)
    {
    case 0:
        printf("[AUT] salir\n");
        // salir=1;
        sprintf(str, "%s", "Closed session.");
        break;
    case 1:
        printf("[AUT] user ls\n");
        list_users(str);
        break;
    case 2:
        printf("[AUT] user passwd\n");
        change_pass(user, get_pass(str));
        sprintf(str, "Password correctly modified.");
        break;
    }

    /*  Send msg to server  */ 
    snd_msg(msqid, str, auth_type);

    // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
    //     fprintf(stderr, "[AUT] Message queue could not be deleted.\n");
    //     exit(EXIT_FAILURE);
    // }

    return status;
}

int main(void){
    /*  Load DB and create
        message queue */
    load_db();
    int msqid = mqid();
    printf("%s\n", "[AUT] Messages queue created...");
    fflush(stdout);
    // int sesion = 0;
    char * user = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    int salir = 1;
    if (LOGIN)
    {
        login_handler(msqid, user);
        printf("[AUT] %s has logged in. \n", user);
        last_connect(user);
        printf("%s\n", "[AUT] Successful login...");
        fflush(stdout);
    }
    int contador = 0;
    do
    {
        printf("[AUT] precmdhandler\n");
        salir = cmd_handler(msqid, user);
        printf("status: %d\n", salir);
        printf("[AUT] postcmdhandler\n");
        contador++;
        if (!salir)
        {
            puts("SALIMOSSSSSSSSSSSS");
        }
        printf("Contador de mensajes: %d\n", contador);
        printf("Exit %d;\n", salir);
    }while(salir!=0);

    printf("Finalice auth.\n");

    // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
    //     fprintf(stderr, "Message queue could not be deleted.\n");
    //     exit(EXIT_FAILURE);
    // }

    return 0;
}

/**
 * @brief 
 * Print users in DB
 */
char * list_users(char * strUsers){
    char * aux = (char*) malloc((BUFFSIZE*USERS+1)*sizeof(char));
    sprintf(strUsers, "%-15s %-15s %10s\n", "Username", "Last connect", "Block");
    for (int i = 0; i < USERS; i++)
    {
        memset(aux,0,strlen(aux));
        sprintf(aux, "%-15s %-15s %10s", users[i].username, users[i].lastconnect, users[i].block);
        strcat(strUsers, aux);
    }
    strcat(strUsers, "\n");
    return strUsers;
}

/**
 * @brief Load users to DB from CSV file
 */
void load_db(){
    char * filename = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char buf[BUFFSIZE];
    // if (chdir("../db") != 0)
    // {
    //     perror("Can't change directory.\n");
    //     exit(EXIT_FAILURE);
    // }
    sprintf(filename, "sample.csv");
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        printf("Can't open file\n");
        exit(EXIT_FAILURE);
    }

    int row_count = 0;
    int field_count = 0;
    while (fgets(buf, BUFFSIZE, fp)) {
        field_count = 0;

        char *field = strtok(buf, ",");

        while (field) {
            if (field_count == 0) {
                strcpy(users[row_count].username, field);
            }
            if (field_count == 1) {
                strcpy(users[row_count].password, field);
            }
            if (field_count == 2) {
                strcpy(users[row_count].lastconnect, field);
            }
            if (field_count == 3) {
                strcpy(users[row_count].block, field);
            }

            if(DEBUG) printf("%s\n", field);
            field = strtok(NULL, ",");

            field_count++;
        }
        if(DEBUG) printf("\n");

        row_count++;
    }

    // if(DEBUG) list_users();
    free(filename);
    fclose(fp);
}

/**
 * @brief Change user pass in DB
 * @param user 
 * @param pass 
 */
void change_pass(char* user, char* pass){
    for (int i = 0; i < USERS; i++)
    {
        if(!strcmp(user, users[i].username)){
            strcpy(users[i].password, pass);
            if(DEBUG) printf("Nueva password: %s\n", users[i].password);
            break;
        }
    }
    save_db();
}

/**
 * @brief 
 * Save DB changes in CSV file
 */
void save_db(){
    FILE *fp;
    fp  = fopen ("sample.csv", "w");
    for (int i = 0; i < USERS; i++)
    {
        fprintf(fp, "%s,%s,%s,%s", users[i].username, users[i].password, users[i].lastconnect, users[i].block);
    }
    fclose (fp);
}

/**
 * @brief 
 * Change user pass in DB
 * @param user 
 * @param pass 
 */
int check_pass(char* user, char* pass){
    int status;
    for (int i = 0; i < USERS; i++)
    {
        if(!strcmp(user, users[i].username)){
            // User exists, password will be checked
            status = (!strcmp(users[i].password, pass)) ? 1 : 0;
            break;
        }
        else
        {
            // User doesn't exists
            status = -1;
        }
    }
    return status;
}