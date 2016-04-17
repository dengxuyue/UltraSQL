%{
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include "scanner.h"
#include "gram.h"
#include "kw_list.c"
%}

%option 8bit reentrant warn
%option bison-bridge bison-locations
%option outfile="scan.c" header-file="scan.h"
%option nodefault noinput nounput noyywrap
%option case-insensitive never-interactive
%option prefix="mini_yy"

space_inline  [ \t]
space         [ \t\r\n\f]
newline       [\r\n]
non_newline   [^\n\r]

semicolons    ;+[ \t;]*

ansi_comment  ("--"{non_newline}*)

white_space   ({space}+|{ansi_comment})

c_comment_start   \/\*[^\*]*
c_comment_inside  [^\*/]*
c_comment_stop    \*+\/

word          [_a-z][_a-z0-9]*
digit         [0-9]*[\.e]?[0-9]+

arith_operator   [\+\-\*\/=><]

leading_dot   ^{space_inline}*\.
 
dquote_string  \"[^\"]+\" 
squote_string  \'[^\"]+\' 

single_char    [\.,\(\)]

%%

{white_space}    {
                 }

{digit}          {
                     SCANNER_DEBUG(yytext, IDENT);
                     return IDENT;
                 }

{word}           {
                     return find_keyword(yytext,yyscanner);
                 }

{arith_operator} {
                     SCANNER_DEBUG(yytext, IDENT);
                     return IDENT;

                 }

{squote_string}  {
                     SCANNER_DEBUG(yytext, IDENT);
                     return IDENT;
                 }

{dquote_string}  {
                     SCANNER_DEBUG(yytext, IDENT);
                     return IDENT;
                 }

{semicolons}     {
                     SCANNER_DEBUG(yytext, SEMICOLON);
                     return SEMICOLON;
                 }

{single_char}    {   
                     return (char)yytext[0];
                 }
.                {
                     fprintf(stderr, "bad input character '%s'\n", yytext);
                 }

%%

yyscan_t scanner_init (const char *str,
                       mini_yy_extra_type *yyext,
                       valid_request *req)
{
    int slen = strlen(str);
    yyscan_t scanner;
    if (yylex_init(&scanner) != 0) 
        fprintf(stderr, "yylex_init() failed.");

    mini_yyset_extra(yyext, scanner);
    yyext->textbuf = (char*)malloc(slen + 2);
    yyext->request = req;

    memcpy(yyext->textbuf, str, slen);
    
    yyext->textbuf[slen] = yyext->textbuf[slen + 1] = YY_END_OF_BUFFER_CHAR;
    yy_scan_buffer(yyext->textbuf, slen + 2, scanner);

    return scanner;
}

void scanner_fini (yyscan_t scanner)
{
    mini_yy_extra_type *yyext = yyget_extra(scanner);

    if (yyext->textbuf)
        free(yyext->textbuf);
}

int find_keyword (char* str, yyscan_t scanner)
{
    int ret = IDENT;
    int kw_list_size = sizeof(mini_kw_list)/sizeof(mini_kw_list[0]);
    mini_yy_extra_type *yyext = yyget_extra(scanner);
    valid_request* req = yyext->request;

    int i;
    for(i = 0; i < kw_list_size; i++ ) {
        if(!strcasecmp(mini_kw_list[i].name, str)) {
            ret = req->subtype = mini_kw_list[i].value;
            break;
        }
    }
    
    SCANNER_DEBUG(str, ret);
    return ret;
}
