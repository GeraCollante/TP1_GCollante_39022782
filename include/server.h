#include "socket.h"
#include "mq.h"
#include "common.h"
#include <sys/types.h>
#include <sys/wait.h>

long    get_1st_char    (char *);
long    cmd_handler     (char *, char *);
void    login_handler   (int, int);
void    rcv_cmd         (int, int);