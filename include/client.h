#include "socket.h"
#include "prompt.h"
#include "common.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>

char *  login           (char *);
int     check_status    (int);
void    login_handler   (int);
void    send_cmd        (int, char *);