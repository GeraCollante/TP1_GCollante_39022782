#include "md5.h"
#include "mq.h"
#include "common.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <errno.h>
#include <string.h>

int     cmd_handler (int);
char *  files_info  (char * );
char *  readable_fs (long int, char *);