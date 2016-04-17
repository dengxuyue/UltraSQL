%{
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include "node.h"
#include "scanner.h"

#define yylex mini_yylex
#define yyerror mini_yyerror

%}

%pure-parser
%expect 0
%locations
%parse-param {mini_yyscan_t yyscanner}
%lex-param   {mini_yyscan_t yyscanner}

%token IDENT SEMICOLON
%token AS BY CREATE DATABASE DELETE DROP FROM GROUP INSERT INTO INDEX ON ORDER PROCEDURE SELECT TABLE 
%token USER VALUES 
%token ABEGIN END
%token MAX MIN COUNT SUM AVG

%%

request: request_stmt SEMICOLON

request_stmt: create_database_stmt
    | create_table_stmt
    | create_index_stmt
    | database_stmt
    | delete_table_stmt
    | drop_database_stmt
    | drop_table_stmt
    | drop_index_stmt
    | insert_stmt
    | select_stmt 
    ;

create_table_stmt: CREATE TABLE DIDENT '(' comma_list ')'
    ;

create_database_stmt: CREATE DATABASE DIDENT AS comma_list
    ;

create_index_stmt: CREATE INDEX DIDENT '(' comma_list ')' ON DIDENT
    ;

database_stmt: DATABASE DIDENT
    ;

delete_table_stmt: DELETE FROM DIDENT
    ;

drop_database_stmt: DROP DATABASE DIDENT
    ;

drop_index_stmt: DROP INDEX DIDENT ON DIDENT
    ;

drop_table_stmt: DROP TABLE DIDENT
    ;

insert_stmt: INSERT INTO DIDENT VALUES '(' comma_list ')'
    | INSERT INTO DIDENT select_stmt
    ;

select_stmt: simple_select_stmt 
    | simple_select_stmt GROUP BY comma_list
    | simple_select_stmt ORDER BY comma_list
    | simple_select_stmt GROUP BY comma_list ORDER BY comma_list
    ;

simple_select_stmt: SELECT comma_list 
    | SELECT comma_list FROM comma_list
    ;

comma_list: words 
    | words ',' comma_list
    ;

words: func_words | simple_words

func_key: MAX | MIN | COUNT | AVG | ABEGIN | END

func_words: func_key simple_words

DIDENT: IDENT | IDENT '.' IDENT

simple_words: DIDENT
    | DIDENT words
    | '(' words ')'
    ;

%%

int mini_yyerror (YYLTYPE *l, mini_yyscan_t m, char const *s)
{
    fprintf(stderr, "%s.\n", s);
    return 1;
}

#include "scan.c"
