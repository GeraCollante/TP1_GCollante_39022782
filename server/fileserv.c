#include "fileserv.h"

int main(void){
    int end = 1, counter=0, msqid;
    /*  Create message queue    */
    msqid = mqid();
    /*  Message queue info  */
    if(DEBUG)   mq_info(msqid);
    printf("[FLS] Messages queue created...\n");
    fflush(stdout);

    /*  Waiting for cmd from [SRV]   */
    do
    {
        /*  Handler of cmd  */
        end = cmd_handler(msqid);
        if(DEBUG)   printf("end: %d\n", end);

        /*  Counter of msg  */
        counter++;
        if(DEBUG)   printf("[FLS] Message counter: %d\n", counter);
    }while(end);

    if(DEBUG)   printf("[FLS] End.\n");

    // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
    //     fprintf(stderr, "Message queue could not be deleted.\n");
    //     exit(EXIT_FAILURE);
    // }

    return 0;
}

/**
 * @brief   Parser that returns in human-readable format the size of a file.
 * @param   size  in bytes from file
 * @param   buf   string
 * @return  Formatted string what contains human-readable size of file.
 */
char* readable_fs(long int size/*in bytes*/, char *buf) {
    int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*ld %s", i, size, units[i]);
    return buf;
}

/**
 * @brief   Collection of the required information for each file in a specific folder.
 * @param   filesInfo 
 * @return  Formatted string ready to be printed or sent
 */
char * files_info(char * filesInfo){
    /*  Pointer */
    DIR *d;
    struct dirent *dir;
    struct stat *buf;

    /*  Allocate memory for structs */
    buf = malloc(sizeof(struct stat));
    dir = malloc(sizeof(struct dirent));

    /*  Allocate memory for strings */
    char * folder   = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * filename = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * fsize    = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * md5      = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * aux      = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    /*  Folder where the images are stored*/
    char path[]  = "img";
    sprintf(folder, "%s", path);
    
    /*  Open folder */
    d = opendir(folder);
    /*  Formatting the table header */
    sprintf(filesInfo, "%-45s %-20s %-20s\n", "Name", "Size", "MD5");
    if (d)
    {
        /*  Iterate each element in folder */
        while ((dir = readdir(d)) != NULL) 
        {
            memset(md5,0,strlen(md5));
            /*  File name */
            sprintf(filename, "%s/%s", folder, dir->d_name);
            
            /*  Stat is a function that returns a pointer to a structure
                that has information about a certain file in its fields. */
            stat(filename, buf);
            
            /*  dir->d_name  = file name
                buf->st_size = file size in bytes
                file_md5     = file md5                */
            /*  Formatting file information */
            sprintf(aux, "%-45s %-20s %-20s\n", dir->d_name, readable_fs(buf->st_size, fsize), file_md5(filename, md5));
            
            /*  Concatenating while iterating each file in the folder */
            strcat(filesInfo, aux);
        }
        closedir(d);
    }
    /* Free memory allocated */
    // free(buf);
    // free(dir);
    // free(folder);
    // free(filename);
    // free(fsize);
    // free(aux);
    return filesInfo;
}


/**
 * @brief   Iterates through all the items in the folder until find the required file.
 * @param   str filename
 * @return  int 
 */
int is_valid_file(char * str){
    /*  Pointers */
    int exist = 0;
    DIR *d;
    struct dirent *dir;

    /*  Allocate memory for structs */
    dir = malloc(sizeof(struct dirent));

    /*  Allocate memory for strings */
    char * folder   = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * filename = (char*) malloc((BUFFSIZE+1)*sizeof(char));

    /*  Folder where the images are stored*/
    char path[]  = "img";
    sprintf(folder, "%s", path);
    
    /*  Open folder */
    d = opendir(folder);

    if(d)
    {
        /*  Iterate each element in folder */
        while ((dir = readdir(d)) != NULL) 
        {
            /*  File name */
            sprintf(filename, "%s/%s", folder, dir->d_name);
            printf("%s\n", dir->d_name);
            printf("%s\n", str);
            if(!strcmp(dir->d_name, str)){
                if(DEBUG)   printf("Existe el archivo. \n");
                exist = 1;
                break;
            }
        }
        closedir(d);
    }
    /* Free memory allocated */
    // free(buf);
    // free(dir);
    // free(folder);
    // free(filename);
    // free(fsize);
    // free(aux);
    return exist;
}

/**
 * @brief   Checks the existence of the image in the folder img.
 * @param   str     string to client
 * @param   file    filename
 * @return  int     tf
 */
int check_file(char * str, char * file){
    /*  Variables declaration   */
    int ft = 0;
    /*  Get filename    */
    strtok(str, ",");
    sprintf(file, "%s", strtok(NULL, ","));

    /*  Check if we have the requested image in img folder  */
    if(is_valid_file(file)==1){
        if(DEBUG)   printf("We will transfer the file. \n");
        ft=1;
    }
    else{
        if(DEBUG)   printf("The file does not exists. \n");
    };

    /*  Copy the result of the verification in the chain that we will 
        send to the client so that they know if the transfer will be made   */
    sprintf(str, "%d", ft);
    return ft;
}

/**
 * @brief   Sockets are created and the file is sent to the client.
 * @param   file 
 */
void transfer_fs(char * file){
    if(DEBUG) printf("Begin transfer file... \n");
    if(DEBUG) printf("File: %s\n", file);

    /*  Create sockets and listening    */
    int socket = srv_socket(PORT_FLS);

    /*  Client will connect for transfer    */
    int connfd = wait_cli(socket);
    send_file(connfd, file);

    /*  File transferred successfully   */
    close(connfd);
    if(DEBUG) printf("End transfer file...\n");
}

/**
 * @brief   Command handler.
 * @param   msqid 
 * @return  int exit
 */
int cmd_handler(int msqid){
    int status, tf=0;
    char * str  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    char * file  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    
    /*  Receive userpass from [SVR] */
    rcv_msg(msqid, str, files_type);
    printf("[FLS]<-[SRV]: %s\n",str);
    fflush(stdout);

    /*  Get status of userpass and convert to int */
    status = atoi(str);

    /*  Decode cmd  */
    switch (status)
    {
    case 1:
        printf("[FLS] file ls\n");
        files_info(str);
        // sprintf(str, "Info files.");
        break;
    case 2:
        printf("[FLS] file down\n");
        tf = check_file(str, file);
        // sprintf(str, "File down.");
        break;
    }

    /*  Send msg to server  */ 
    snd_msg(msqid, str, files_type);

    /*  If the required file exists we proceed 
        to create a socket to send it */
    if(tf)  transfer_fs(file);

    // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
    //     fprintf(stderr, "[FLS] Message queue could not be deleted.\n");
    //     exit(EXIT_FAILURE);
    // }
    // if(tf)
    // {
    // }

    return status;
}

//TODO  Hacer free de casi todo
//TODO  MBR
//TODO  Reiniciar todo
//TODO  Latex
//TODO  Burn USB