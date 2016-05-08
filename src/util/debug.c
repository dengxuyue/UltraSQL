/*
 * util/debug.c
 *
 * Copyright 2011 Xuyue Deng
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include "debug.h"

int trace_level = 0;

static FILE* trace = NULL;

int init_trace()
{
	trace = NULL;

    char def_trace[32];
    char* login = getlogin();
    snprintf(def_trace, 31, "/tmp/usql.trace.%s", (login ? login : "me") );

	char* penv = getenv("USQL_TRACE");

	char* pfn;
	if(penv)
		pfn = penv;
	else
		pfn = def_trace;

	if ( (trace = fopen(pfn, "a+t")) != NULL ) {
		return 0;
	}

	return 1;
}

void dump_trace(int level, char* component, char* fmt, ...)
{
    if (!level || !trace_level || !trace)
        return;

	if (trace_level <= level) {
		fprintf(trace, "[%s]: ", component);
        va_list ap;
        va_start(ap, fmt);
        vfprintf(trace, fmt, ap);
        va_end(ap);
        fprintf(trace, "\n");
	}
}

void fini_trace()
{
	if (trace) {
		fclose(trace);
		trace = NULL;
	}
}
