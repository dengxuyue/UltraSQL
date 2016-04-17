#ifndef _HELPER_H_
#define _HELPER_H_
#include <stdio.h>

typedef void (*ts_sigfunc) (int);

ts_sigfunc ts_signal(int signo, ts_sigfunc func);
int ts_getpass(char* lineptr, int n, FILE* stream);

#endif
