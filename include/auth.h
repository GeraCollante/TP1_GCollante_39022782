#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define USERS 3
#define DEBUG 0

struct user
{
    char username[BUFFSIZE];
    char password[BUFFSIZE];
    char lastconnect[BUFFSIZE];
    char block[BUFFSIZE];
};

struct user users[USERS];
// int usuarios = 3;
// struct user* array = malloc(usuarios * sizeof(struct user));

void load_db();
void change_pass(char *, char *);
void save_db();