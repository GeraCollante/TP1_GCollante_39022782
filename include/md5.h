#include <errno.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

//TODO comentar md5

#define BUFFSIZE 1024

char *  file_md5    (char *, char *);
char *  get_md5     (char *, ssize_t, char *);