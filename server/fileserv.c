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
 * @brief   Parser that returns in human-readable format the size of a file
 * @param   size  in bytes from file
 * @param   buf   string
 * @return  Formatted string what contains human-readable size of file
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

int cmd_handler(int msqid){
    int status;
    // int end = 0;
    char * str  = (char*) malloc((BUFFSIZE+1)*sizeof(char));
    
    /*  Receive userpass from [SVR] */
    rcv_msg(msqid, str, files_type);
    printf("[FLS]<-[SRV]: %s\n",str);
    fflush(stdout);

    /*  Get status of userpass and convert to int */
    status = atoi(str);
    // printf("predecoder\n");
    // cmd_decoder(status, str);
    // printf("postdecoder\n");
    
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
        // sprintf(str, "File down.");
        break;
    }

    /*  Send msg to server  */ 
    snd_msg(msqid, str, files_type);

    // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
    //     fprintf(stderr, "[FLS] Message queue could not be deleted.\n");
    //     exit(EXIT_FAILURE);
    // }

    return status;
}