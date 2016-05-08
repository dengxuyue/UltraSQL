#include "miniparser.h"
#include <string.h>

static int search_for_dot (char* text)
{
    int i, len = strlen(text);
    for (i = 0; i < len; i++) {
        if (*(text + i) != ' ' && *(text + i) != '\t') {
            if (*(text + i) == '.')
                return 1;
            else
                return 0;
        }
    }

    return 0;
}


enum {
    left_paren$ = 1,
    left_c_comment$,
    ansi_comment$,
    double_quote$,
    single_quote$,
};

static int search_for_statement(char* text)
{
    unsigned char status[STATEMENT_NEST_LEVEL];
    int ind = 0 /* index in above array 'status' */;
    int ended = 0 /* the statement is ended */;
    int i, len = strlen(text);
    for (i = 0; i < len; i++) {
        if (*(text + i) == ' ' || *(text + i) == '\t' ) {
        }
        else if (*(text + i) == '\n' || *(text + i) == '\r') {
            if (ind > 0 && status[ind - 1] == ansi_comment$)
                --ind;
        }
        else if (*(text + i) == '(') {
            if (ind == 0 || (status[ind - 1] != ansi_comment$ ||
                             status[ind - 1] != left_c_comment$ ||
                             status[ind - 1] != double_quote$ ||
                             status[ind - 1] != single_quote$))
                status[ind++] = left_paren$;
            ended = 0;
        }
        else if (*(text + i) == ')') {
            if (ind > 0 && status[ind - 1] == left_paren$)
                --ind;
            ended = 0;
        }
        else if (*(text + i) == '\'') {
            if (ind > 0) {
                if (status[ind - 1] == single_quote$)
                    --ind;
                else if (status[ind - 1] != ansi_comment$ &&
                         status[ind - 1] != left_c_comment$ &&
                         status[ind - 1] != double_quote$)
                    status[ind++] = single_quote$;
            }
            else
                status[ind++] = single_quote$;
            ended = 0;
        }
        else if (*(text + i) == '"') {
            if (ind > 0) {
                if (status[ind - 1] == double_quote$)
                    --ind;
                else if (status[ind - 1] != ansi_comment$ &&
                         status[ind - 1] != left_c_comment$ &&
                         status[ind - 1] != single_quote$)
                    status[ind++] = double_quote$;
            }
            else
                status[ind++] = double_quote$;
            ended = 0;
        }
        else if (*(text + i) == '/') {
            if (ind > 0) {
                if (status[ind - 1] == ansi_comment$ ||
                    status[ind - 1] == single_quote$ ||
                    status[ind - 1] == double_quote$){
                }
                else if (status[ind - 1] == left_c_comment$) {
                    if (i > 0 && *(text + i - 1) == '*')
                        --ind;
                }
                else if (i < len && *(text + i + 1) == '*') {
                    status[ind++] = left_c_comment$;
                    i += 2;
                }
            }
            else if (i < len && *(text + i + 1) == '*') {
                status[ind++] = left_c_comment$;
                i += 2;
            }
            ended = 0;
        }
        else if (*(text + i) == ';' && ind == 0)
            ended = 1;
        else
            ended = 0;

        if(ind >= STATEMENT_NEST_LEVEL)
            return 1;
    }

    return ind == 0 && ended == 1 ? 1 : 0;
}


int coarse_check(char* text, int end_delimiter)
{
    if (search_for_dot(text))
        return DOT_DIRECTIVE;

    if (end_delimiter)
        return END_DELIMITER;

    if (search_for_statement(text))
        return SQL_STATEMENT;

    return 0;
}

