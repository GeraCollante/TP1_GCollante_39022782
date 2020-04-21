#include "socket.h"
#include "mq.h"
#include "common.h"
#include <sys/types.h>
#include <sys/wait.h>

long    cmd_handler     (char *, char *);
long    get_1st_char    (char *);
void    login_handler   (int, int);
void    rcv_cmd         (int, int);