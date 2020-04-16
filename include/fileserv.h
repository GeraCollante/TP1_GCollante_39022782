#include "md5.h"
#include "mq.h"
#include "common.h"
#include "socket.h"
#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <openssl/md5.h>
#include <errno.h>

int     check_file      (char *, char *);
int     cmd_handler     (int);
char *  files_info      (char *);
int     is_valid_file   (char *);
char *  readable_fs     (long int, char *);
void    transfer_fs     (char *);