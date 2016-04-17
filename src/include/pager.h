#ifndef __PAGER_H__
#define __PAGER_H__

#include <stdio.h>
#include "helper.h"

extern FILE *internal_pager;
extern int  output_screen_width;

void open_resp_pager ();
void close_resp_pager ();
ts_sigfunc signal_resp_pager (ts_sigfunc int_pager);
void set_resp_width(int w);


#endif /* __PAGER_H__ */
