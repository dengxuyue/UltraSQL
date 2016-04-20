#include "miniparser.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "scanner.h"
#include "gram.h"
#include "scan.h"
#include "debug.h"

char internal_delimiter[SET_DELIMITER_LENGTH];

mini_dots_t mini_dots_list[] = {
    {"logon",   5, DTD_LOGON} 
   ,{"logoff",  6, DTD_LOGOFF}
   ,{"quit",    4, DTD_QUIT}
   ,{"set",     3, DTD_SET}
   ,{"session", 7, DTD_SESSION}
};

static int  internal_sqlsyntax_on  = 1;
static int  internal_sqlcompliance = SQL_COMPLIANCE_DEFAULT;

int init_parser ()
{
    internal_delimiter[0] = '\0';
    return 0;
}


void fini_parser ()
{
    return;
}


void reset_parser ()
{
    internal_delimiter[0] = '\0';
    return;
}


void set_parser_delimiter (char* delimiter)
{
    int len = strlen(delimiter);
    if (len) 
        strncpy(internal_delimiter, delimiter, len);
    internal_delimiter[len] = '\0';
}


void set_parser_sqlsyntax (int on) 
{
    if (on)
        internal_sqlsyntax_on = 1;
    else
        internal_sqlsyntax_on = 0;
}


void set_parser_sqlcompliance (int compliance) 
{
    switch (compliance) {
    case SQL_COMPLIANCE_DEFAULT:
    case SQL_COMPLIANCE_ANSI:
    case SQL_COMPLIANCE_TERADATA:
    case SQL_COMPLIANCE_POSTGRESQL:
    case SQL_COMPLIANCE_MYSQL:
        internal_sqlcompliance = compliance;
        break;
    default:
        internal_sqlcompliance = SQL_COMPLIANCE_DEFAULT;
        break;
    }
}


int parse (char* text, valid_request *req) 
{
    MINIPARSER_DEBUG_C();

    int has_end_delimiter = strlen(internal_delimiter) ? 1 : 0;
    int st = coarse_check(text, has_end_delimiter);
    if (st == DOT_DIRECTIVE)
        return parse_dot_directive(text, req);
    else if (st == END_DELIMITER) 
        return parse_end_delimiter(text, req);
    else if (st == SQL_STATEMENT) 
        return parse_sql_statement(text, req);

    return PARSE_CONT;
}


int parse_end_delimiter (char* text, valid_request *req) 
{
    MINIPARSER_DEBUG_C();

    int len = strlen(text), dlen = strlen(internal_delimiter);
    int i, start = 0, end = len - 1;
    for(i = len - 1; i >= 0; i--) {
        if (!(*(text + i) == '\t' || *(text + i) == ' '  ||
              *(text + i) == '\n' || *(text + i) == '\r' ||
              *(text + i) == ';')) {
            end = i;
            break;
        }
    }
    for(i = end - 1; i >= 0; i--) {
        if ((*(text + i) == '\t' || *(text + i) == ' '  ||
             *(text + i) == '\n' || *(text + i) == '\r' ||
             *(text + i) == ';')) {
            start = i + 1;
            break;
        }
    }

    if ( end - start + 1 < dlen)
        return PARSE_CONT;
   
    if (strncmp(internal_delimiter, text + start, dlen)) 
        return PARSE_CONT;

    if (start + 1 > STATEMENT_LENGTH_LIMIT) 
        req->validtext = (char*)malloc(start + 1);
    else 
        req->validtext = req->prealloc_text;
    strncpy(req->validtext, text, start);
    req->validtext[start] = '\0';

    return PARSE_VALID;
}


int parse_dot_directive (char* text, valid_request *req)
{
    MINIPARSER_DEBUG_C();

    int i, len = strlen(text);
    char *ptr = text;
    int found_dot = 0;
    for (i = 0; i < len; i++) {
        if (!(*ptr == ' ' || *ptr == '\t' || *ptr == '.')) 
            break;
        
        ptr++;
    }
    
    req->type = DOT_DIRECTIVE;

    int dots_list_size = sizeof(mini_dots_list)/sizeof(mini_dots_list[0]);

    for(i = 0; i < dots_list_size; i++ ) {
        if(!strncasecmp(ptr, mini_dots_list[i].name, mini_dots_list[i].length)) {
            req->subtype = mini_dots_list[i].value;
            MINIPARSER_DEBUG("Get dot directive %s", ptr);
            MINIPARSER_DEBUG("Dot directive subtype is %d", req->subtype);
            ptr += mini_dots_list[i].length;
            break;
        }
    }
 
    if (!req->subtype) {
        fprintf(stderr, "Syntax error, unknown dot directive.\n");
        return PARSE_INVALID;
    }

    if (req->subtype == DTD_QUIT)
        return PARSE_QUIT;

    if (req->subtype == DTD_LOGON ) {
        if (parse_logon(ptr, req)) 
            return PARSE_INVALID;
    }
    else if (req->subtype == DTD_SET ) {
        if (parse_set(ptr, req)) 
            return PARSE_INVALID;
    }
    else if (req->subtype == DTD_SESSION ) {
        if (parse_session(ptr, req)) 
            return PARSE_INVALID;
    }

    return PARSE_VALID;
}


int parse_sql_statement (char* text, valid_request *req)
{
    MINIPARSER_DEBUG_C();

    mini_yy_extra_type yyextra_data;
    yyscan_t yyscanner = NULL;
    int len = strlen(text);
    int yyresult = 0, ret = 0;

    if (internal_sqlsyntax_on) {
        yyscanner = scanner_init(text, &yyextra_data, req);
        init_parser();
        yyresult = yyparse(yyscanner);

        if (yyresult)
            ret = PARSE_INVALID;
        else
            ret = PARSE_VALID;
        
        fini_parser();
        scanner_fini(yyscanner);
    }
    else
        ret = PARSE_VALID;

    if (ret == PARSE_VALID) {
        if (len + 1 > STATEMENT_LENGTH_LIMIT) 
            req->validtext = (char*)malloc(len + 1);
        else 
            req->validtext = req->prealloc_text;
        strncpy(req->validtext, text, len);
        req->validtext[len] = '\0';
    }
    
    if (yyresult == 2) {
        fprintf(stderr, "Insufficient memory.\n");
        exit(1);
    }
        
    return ret;
}


int free_request(valid_request *req)
{
    if (req->validtext && req->validtext != req->prealloc_text)
        free(req->validtext);

    req->type = 0;
    req->subtype = 0;
    req->validtext = 0;

    return 0;
}

