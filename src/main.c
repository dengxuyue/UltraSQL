/*
 * Main file
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "debug.h"
#include "miniparser.h"
#include "dbi.h"
#include "cmdhistory.h"
#include "profile.h"
#include "helper.h"


/*
 * All the information got or derived from the response of database should print
 * through STDOUT
 * The program exceptions can print through STDERR.
 * 
 * " -*- " informative message prefix
 * " -!- " warning/error message prefix
 *
 */

/*
 * Global options
 */
extern int trace_level;

static void set_trace_level (char* next_argv) 
{
    if (*next_argv == '-') 
        /* Another argument */
        trace_level = 1;
    else if (*next_argv == '2') 
        trace_level = 2;
    else if (*next_argv == '3') 
        /*TODO: provide trace_level 3*/
        trace_level = 3;
    else 
        trace_level = 1;
}

int get_option (int argc, char** argv)
{
    int i;
    char *ptr;
    int is_option;
    for (i = 1; i < argc; i++) {
        ptr = argv[i];
        is_option = 0;
        if(*ptr == '-') {
            /* short option: 1 */
            is_option++;
            ptr++;
        }
        if(*ptr == '-') {
            /* long option: 2 */
            is_option++;
            ptr++;
        }

        if (is_option == 1) {
            if (*ptr == 'm' && *(ptr + 1) == '\0') {
                enable_multi_session();
            }
            else if (*ptr == 'd' && *(ptr + 1) == '\0') {
                if (i + 1 < argc) {
                    set_trace_level(argv[i + 1]);
                }
                else {
                    trace_level = 1;
                }
            }
        }
        else if (is_option == 2) {
            if(!strcmp(ptr, "multi-session")) {
                enable_multi_session();
            }
            else if (!strcmp(ptr, "debug")) {
                if (i + 1 < argc) {
                    set_trace_level(argv[i + 1]);
                }
                else {
                    trace_level = 1;
                }
            }
        }
    }
}


int is_null_line(char* line)
{
    char *p = line;
    while (*p != '\0') {
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') 
            continue;
        else 
            return 0;
        p++;
    }

    return 1;
}


int exec_dot_set (valid_request * req, int quiet)
{
    if (!req || req->type != DOT_DIRECTIVE || req->subtype != DTD_SET )
        return -1;

    switch(req->node.set.type) {
    case DTD_SET_WIDTH: 
        set_resp_width(req->node.set.value.width);
        if (!quiet)
            fprintf(stdout, "-*- You have set new default width.\n\n");
        break;

    case DTD_SET_DELIMITER:
        set_parser_delimiter(req->node.set.value.delimiter);
        if (!quiet)
            fprintf(stdout, "-*- You have set request delimter.\n\n");
        break;

    case DTD_SET_SQLSYNTAX:
        set_parser_sqlsyntax(req->node.set.value.sqlsyntax);
        if (!quiet)
            fprintf(stdout, "-*- You have set sql syntax check %s.\n\n", 
                req->node.set.value.sqlsyntax ? "on" : "off");
        break;

    case DTD_SET_SQLCOMPLIANCE:
        set_parser_sqlcompliance(req->node.set.value.sqlcompliance);
        if (!quiet)
            fprintf(stdout, "-*- You have set sql compliance.\n\n");
        break;

    case DTD_SET_PROTOCAL:
        set_parser_protocal(req->node.set.value.protocal);
        if (!quiet)
            fprintf(stdout, "-*- You have set connectivity protocal.\n\n");
        break;

    case DTD_SET_SIDETITLES:
        /* set_parser_protocal(req->node.set.value.sidetitles); */
        if (!quiet)
            fprintf(stdout, "-*- You have turn %s side titles.\n\n",
                req->node.set.value.sidetitles ? "on" : "off");
        break;

    default:
        fprintf(stderr, "-!- You have input an unknown command.\n\n");
        break;
    }

    return 0;
}

void int_fini (int sig) 
{
    fini_history(16);
    fini_trace();
}

int main (int argc, char** argv) 
{
    get_option(argc, argv);

    fprintf(stdout, "        |-----------------------|\n");
    fprintf(stdout, "        |      UltraSQL 0.2     |\n");
    if (trace_level)
    fprintf(stdout, "        | ::Trace Level (%d)     |\n", trace_level);
    fprintf(stdout, "        |-----------------------|\n\n");
    
    valid_request req = {
        .type      = 0,
        .subtype   = 0,
        .validtext = 0,
    };

    dbi_preinit_all();
    {
        set_parser_protocal("pq");
    }

    init_trace();
    init_history();
    init_resp_buffer();
    init_profile();

    /* Catch INT */
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_sigaction = &int_fini;
    act.sa_flags |= SA_RESETHAND;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        fprintf(stdout, "-!- Sorry, I cannot catch SIGINT.\n");
    }

    int stmt_text_offset = 0, stmt_text_length = STATEMENT_LENGTH_LIMIT;
    char* stmt_text = (char*)malloc(stmt_text_length);
    if(!stmt_text)
        return 1;

    char *set_item;
    while (set_item = get_next_setting_item()) {
        if (parse(set_item, &req) != PARSE_VALID)
            continue;
        if (req.type == DOT_DIRECTIVE && req.subtype == DTD_SET ) {
            exec_dot_set(&req, 1);
        }
        free_request(&req);
    }
    
    int logon = 0, history_lines = 0;
    int pstatus = PARSE_VALID;
    while (1) {
        char *line = readline("# ");
        if (pstatus == PARSE_VALID && is_null_line(line)) 
            continue;

        int slen = strlen(line);
        /* 2: one is for previous carriage, and one for next '\0' */
        if (stmt_text_offset + slen + 2 > STATEMENT_LENGTH_LIMIT) {
            stmt_text_length += STATEMENT_LENGTH_LIMIT;
            stmt_text = (char*)realloc(stmt_text, stmt_text_length);
            if(!stmt_text)
                return 1;
        }

        if (stmt_text_offset)
            stmt_text[stmt_text_offset++] = ' ';
        strncpy(stmt_text + stmt_text_offset, line, slen);
        stmt_text_offset += slen;
        stmt_text[stmt_text_offset] = '\0';
        pstatus = parse(stmt_text, &req);

        if (pstatus == PARSE_INVALID) {
            stmt_text_offset = 0;
        }
        else if (pstatus == PARSE_QUIT) {
            fprintf(stdout, "-*- See you later.\n");
            break; /*go out of while loop*/
        }
        else if (pstatus == PARSE_VALID) {
            add_history(stmt_text);
            history_lines++;
            if (req.type == DOT_DIRECTIVE && req.subtype == DTD_SET ) {
                exec_dot_set(&req, 0);
            }
            else if (req.type == DOT_DIRECTIVE && req.subtype == DTD_LOGON ) {
                if (dbi_init(req.node.logon.protocal)) 
                    fprintf(stdout, "-!- Fatal errors occur in the session initialization phase.\n\n");

                char  passwd_input[LOGON_PASSWORD_LENGTH];
                char* passwd;
                passwd = get_password(req.node.logon.datasource, req.node.logon.username);
                if(!passwd) { 
                    fprintf(stdout, "Password: ");
                    ts_getpass(passwd_input, LOGON_PASSWORD_LENGTH, stdin);
                    passwd = passwd_input;
                    fprintf(stdout, "\n");
                }
                if(strlen(passwd) + 1 < LOGON_PASSWORD_LENGTH)
                    strcpy(req.node.logon.password, passwd);
                else
                    fprintf(stderr, "syntax error: password is too long: %d.", strlen(passwd) + 1);

                char conn_str[CONN_INFO_STRING_LENGTH];
                get_connection_info(conn_str, CONN_INFO_STRING_LENGTH, &(req.node.logon));

                if (!dbi_connect(conn_str, 128)) { 
                    logon = 1;
                    dbi_fetch();
                    dbi_finish();
                    fprintf(stdout, "-*- You have logged on.\n\n");
                }
                else 
                    fprintf(stdout, "-!- Errors occur when you login the dababase.\n\n");
            }
            else if (req.type == DOT_DIRECTIVE && req.subtype == DTD_LOGOFF ) {
                dbi_end();
            }
            else if (req.type == DOT_DIRECTIVE && req.subtype == DTD_SESSION ) {
                if (req.node.session.type == DTD_SESSION_SHOW) 
                    dbi_session_show(req.node.session.value.alias); 
                else if  (req.node.session.type == DTD_SESSION_SELECT) 
                    dbi_session_select(req.node.session.value.alias); 
            }
            else if (req.type = SQL_STATEMENT ) {
                if (logon) {
                    dbi_execute(req.validtext);
                    dbi_fetch();
                    dbi_finish();
                }
                else {
                    fprintf(stdout, "-!- You should log on at first and then submit request.\n\n");
                }
            }

            free_request(&req);
            stmt_text_offset = 0;
        }
    }

    dbi_end_all();
    dbi_deinit_all();

    free_request(&req);
    if(stmt_text)
        free(stmt_text);

    fini_profile();
    fini_resp_buffer();
    fini_history(history_lines);
    fini_trace();

    return 0;
}

