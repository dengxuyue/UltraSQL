/*
 * include/debug.h
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

extern int trace_level;

#define TRACE_LEVEL_1 (trace_level == 1)
#define TRACE_LEVEL_2 (trace_level == 2)

int  init_trace();
void fini_trace();
void dump_trace(int level, char* component, char *fmt, ...);

#define MINIPARSER_DEBUG_C()   dump_trace(1, "miniparser", "Beginning at file %s (%s)", __FILE__, __func__) 
#define MINIPARSER_DEBUG_S(s)  dump_trace(2, "miniparser", s)
#define MINIPARSER_DEBUG(s, t) dump_trace(2, "miniparser", s, t) 

#define INTERFACE_DEBUG_C()    dump_trace(1, "interface ", "Beginning at file %s (%s)", __FILE__, __func__) 
#define INTERFACE_DEBUG_S(s)   dump_trace(2, "interface ", s) 
#define INTERFACE_DEBUG(s, t)  dump_trace(2, "interface ", s, t)

#endif
