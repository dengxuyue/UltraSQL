#ifndef _MINIPARSER_H_
#define _MINIPARSER_H_

#include "config.h"
#include "node.h"

#define PARSE_VALID     0
#define PARSE_INVALID  -1
#define PARSE_QUIT      1
#define PARSE_CONT      2

typedef struct mini_dots_t {
    char* name;
    int   length;
    int   value;
} mini_dots_t;

int  init_parser ();
void fini_parser ();
void reset_parser ();
void set_parser_delimiter (char* text);
void set_parser_sqlsyntax (int on);
void set_parser_sqlcompliance (int compliance);
int  coarse_check (char* text, int end_delimiter);
int  parse_logon  (char* text, valid_request* req);
int  parse_set    (char* text, valid_request* req);
int  parse_show   (char* text, valid_request* req);
int  parse        (char* text, valid_request *req);

int  free_request (valid_request *req);

#endif /*_MINIPARSER_H_*/
