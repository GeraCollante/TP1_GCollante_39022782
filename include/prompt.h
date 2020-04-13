#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LSH_RL_BUFSIZE  1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM   " \t\r\n\a"

int     argc        (char **);
char *  cmd_prompt  (char *);
int     get_cmd     (char ** , int );
int     is_exit     (char *);
char *  read_line   (void);
char ** split_line  (char *);