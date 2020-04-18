#include "md5.h"
#include "common.h"
//TODO comentar md5

/**
 * @brief   Checksum MD5 of filename
 * @param   filename 
 * @param   md5 
 * @return  Formatted string of md5 checksum of file
 */
char* file_md5(char * filename, char * md5){
    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *fp = fopen (filename, "rb");
    MD5_CTX mdContext;
    size_t bytes;
    unsigned char data[BUFFSIZE*4];
    char temp[20];

    if (fp == NULL){
        perror("File can't be opened.\n");
        exit(EXIT_FAILURE);
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, BUFFSIZE*4, fp)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(temp, "%02x", c[i]);
        strcat(md5, temp);
    }
    // for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    fclose (fp);    
    return md5;
}

/**
 * @brief   Get the md5 object
 * @param   file_path 
 * @param   end 
 * @return  char* 
 */
char* get_md5(char* file_path, ssize_t end, char * md5string){

    sync();

    FILE* file = fopen(file_path, "rb");	// rb para archivos de no-texto;
    char* buffer[BUFFSIZE];
    ssize_t acc = 0;
    size_t n;
    unsigned char c[MD5_DIGEST_LENGTH];
    MD5_CTX mdContext;

    MD5_Init(&mdContext);

    while (acc<end) {
    n = fread (buffer, sizeof(char), sizeof(buffer), file);
    MD5_Update(&mdContext, buffer, n);
    acc += (ssize_t) n;
    }

    MD5_Final (c,&mdContext);
    fclose(file);

    // char* md5string = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    for (int32_t i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&md5string[i * 2], "%02x", (unsigned int)c[i]);

    md5string[strlen(md5string)] = '\0';

    return md5string;
}

//TODO  size_t
//TODO  signal
//TODO  create mq