#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define USERS 3

struct user
{
    char username   [BUFFSIZE];
    char password   [BUFFSIZE];
    char lastconnect[BUFFSIZE];
    char block      [BUFFSIZE];
};

struct user users[USERS];

void    change_pass     (char *, char *);
int     check_block     (char *);
int     cmd_handler     (int, char *);
void    increase_block  (char *);
char *  get_pass        (char *);
int     get_status      (char *, char *);
void    last_connect    (char *);
char *  list_users      (char *);
void    load_db         (void);
char *  login_handler   (int, char *);
void    save_db         (void);