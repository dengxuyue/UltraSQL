#include "config.h"
#include "pager.h"
#include "helper.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

FILE* internal_pager      = 0;
int   output_screen_width = DEFAULT_SCREEN_WIDTH;

void open_resp_pager ()
{
    if (internal_pager)
        return;
    internal_pager = popen(DEFAULT_SCREEN_PAGER, "w");
}


void close_resp_pager ()
{
    if (!internal_pager)
        return;
    pclose(internal_pager);
    internal_pager = 0;
}


ts_sigfunc signal_resp_pager (ts_sigfunc int_pager)
{
    if (!internal_pager)
        return SIG_ERR;

    return ts_signal(SIGPIPE, int_pager);
}


void set_resp_width(int w)
{
    if (w > 0 && w < 65536) 
        output_screen_width = w;
}


