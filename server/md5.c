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