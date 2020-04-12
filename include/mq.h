#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>

#define PERMS       0644
#define BUFFSIZE    1024

struct my_msgbuf {
   long mtype;
   char mtext[BUFFSIZE];
};

int      mqid();
char *   rcv_msg(int, char *, long);
void     snd_msg(int, char *, long);
void     mq_info(int msqid);