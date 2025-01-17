#include "socket.h"
#include "prompt.h"
#include "common.h"
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

void    burn_usb        (char *, char *);
int     check_status    (int);
int     cmd_handler     (int, char *);
void    ctrl_c          (void);
void    file_down       (void);
char *  login           (char *);
void    login_handler   (int);
void    signal_init     (void);
void    sigint_handler  (int);