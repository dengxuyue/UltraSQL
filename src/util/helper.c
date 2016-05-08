#include "helper.h"
#include <signal.h>
#include <termios.h>

ts_sigfunc ts_signal(int signo, ts_sigfunc func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (signo != SIGALRM)
        act.sa_flags |= SA_RESTART;

    if (signo == SIGCHLD)
        act.sa_flags |= SA_NOCLDSTOP;

    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;

    return oact.sa_handler;
}


int ts_getpass(char *lineptr, int n, FILE *stream)
{
    struct termios old, new;
    int nread;

    /* Turn echoing off and fail if we can't. */
    if (tcgetattr (fileno (stream), &old) != 0)
      return -1;
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr (fileno (stream), TCSAFLUSH, &new) != 0)
      return -1;

    /* Read the password. */
    nread = getline (&lineptr, &n, stream);
    char *p = lineptr + nread - 1;
    if (*p == '\n'  || *p == '\r') {
        nread--;
        *p = '\0';
    }

    /* Restore terminal. */
    (void) tcsetattr (fileno (stream), TCSAFLUSH, &old);

    return nread;
}
