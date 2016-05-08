#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "config.h"

typedef struct mini_yy_extra_type {
    char* textbuf;
    valid_request *request;
} mini_yy_extra_type;

typedef struct mini_kw_t {
    char* name;
    int   value;
} mini_kw_t;

typedef void* mini_yyscan_t;

#define SCANNER_DEBUG(s, t) dump_trace(2, "scanner ", "Current text:'%s' | Token: %d", s, t)
#define SCANNER_DEBUG_S(s)  dump_trace(2, "scanner ", "{%s}", s)

int find_keyword (char* str, mini_yyscan_t scanner);

#endif /* _SCANNER_H_ */
