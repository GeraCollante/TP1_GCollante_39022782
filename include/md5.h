#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>

//TODO comentar md5

#define BUFFSIZE 1024

char *  get_md5     (char *, ssize_t, char *);
char *  file_md5    (char *, char *);