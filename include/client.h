#include "socket.h"
#include "prompt.h"
#include "common.h"
#include <errno.h>
#include <termios.h>
#include <unistd.h>

void    burn_usb        (char *, char *);
int     check_status    (int);
void    file_down       (void);
char *  login           (char *);
void    login_handler   (int);
void    send_cmd        (int, char *);