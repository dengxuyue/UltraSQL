#include "dbi.h"
#ifdef POSTGRESQL_SUPPORT
#include "pqlib.h"
#endif
#ifdef MYSQL_SUPPORT
#include "myclient.h"
#endif
#ifdef TDCLI_SUPPORT
#include "tdcli.h"
#endif
#ifdef ODBC_SUPPORT
#include "odbc.h"
#endif
#include "debug.h"
#include <string.h>

static conn_session global_session_list[CONN_SESSION_ENTRY_LIMIT];

/* current active session index in the global session list */
/* it can be chosen by void dbi_session_select (char* alias) */
static int active_session_index = 0;

static int multi_session_mode   = 0;


void enable_multi_session()
{
    multi_session_mode = 1;
}


static inline int session_type (int prot) {
    int type = 0;
    switch (prot) {
    case DBI_PROTOCAL_TDCLI:
        type = TDCLI_SESSION; break;
    case DBI_PROTOCAL_ODBC:
        type = ODBC_SESSION; break;
    case DBI_PROTOCAL_POSTGRESQL:
        type = PQLIB_SESSION; break;
    case DBI_PROTOCAL_MYSQL:
        type = MYSQL_SESSION; break;
    default:
        type = 0;
    }
    return type;
}


int get_connection_info(char* conninfo, int infolen, logon_node* logon)
{
    if(!conninfo || !logon)
        return -1;

    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef TDCLI_SUPPORT
    else if (global_session_list[active_session_index].type == TDCLI_SESSION) {
        snprintf(conninfo, infolen, "%s/%s,%s",
                 logon->datasource,
                 logon->username,
                 logon->password);

    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[active_session_index].type == PQLIB_SESSION) {
        char *ptr = conninfo;
        int  nchar;
        if (strlen(logon->datasource)) {
            nchar = snprintf(ptr, infolen, "host=%s ", logon->datasource);
            ptr += nchar;
            infolen -= nchar;
        }

        if (strlen(logon->dbname)) {
            nchar = snprintf(ptr, infolen, "dbname=%s ", logon->dbname);
            ptr += nchar;
            infolen -= nchar;
        }

        if (strlen(logon->port)) {
            nchar = snprintf(ptr, infolen, "port=%s ", logon->port);
            ptr += nchar;
            infolen -= nchar;
        }
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[active_session_index].type == MYSQL_SESSION) {
        char *ptr = conninfo;
        int nchar;
        nchar = snprintf(ptr, infolen, "%s;%s;%s",
                         logon->datasource,
                         logon->username,
                         logon->password);
        ptr += nchar;
        infolen -= nchar;

        if (strlen(logon->dbname)) {
            nchar = snprintf(ptr, infolen, ";%s", logon->dbname);
        }
        else {
            nchar = snprintf(ptr, 2, ";");
        }
        ptr += nchar;
        infolen -= nchar;

        if (strlen(logon->port))
            nchar = snprintf(ptr, infolen, ";%s", logon->port);
        else
            nchar = snprintf(ptr, 2, ";");
        ptr += nchar;
        infolen -= nchar;

        if (strlen(logon->socket))
            nchar = snprintf(ptr, infolen, ";%s", logon->socket);
        else
            nchar = snprintf(ptr, 2, ";");
        ptr += nchar;
        infolen -= nchar;

    }
#endif
    else if (global_session_list[active_session_index].type == ODBC_SESSION) {
        snprintf(conninfo, infolen, "DSN=%s;", logon->datasource);
    }

    strncpy(global_session_list[active_session_index].conn_info,
            conninfo, CONN_INFO_STRING_LENGTH);

    return 0;
}


static void print_session_detail (int index)
{
    fprintf(stdout, "DBI type   : ");
    switch(global_session_list[index].type) {
    case TDCLI_SESSION:
        fprintf(stdout, "Teradata\n");
        break;
    case PQLIB_SESSION:
        fprintf(stdout, "PostgreSQL\n");
        break;
    case MYSQL_SESSION:
        fprintf(stdout, "MySQL\n");
        break;
    default:
        fprintf(stdout, "Unknown\n");
        break;
    }

    fprintf(stdout, "DBI alias  : %s\n", global_session_list[index].alias);
    fprintf(stdout, "DBI connstr: %s\n\n", global_session_list[index].conn_info);
}


void dbi_session_show (char* alias)
{
    int len = strlen(alias);

    if (!len) {
        if (global_session_list[active_session_index].type) {
            fprintf(stdout, "-*- You want to show current session details.\n\n");
            print_session_detail(active_session_index);
        }
        else {
            fprintf(stdout, "-!- There is no any session existing.\n\n");
        }
        return;
    }

    int show_all = 0;
    if(!strncasecmp(alias, "all", 3))
        show_all = 1;

    int i, len2, show_any = 0;
    for (i = 0; i < CONN_SESSION_ENTRY_LIMIT; i++) {
        if(global_session_list[i].type) {
            len2 = strlen(global_session_list[i].alias);
            if(show_all || !strncasecmp(global_session_list[i].alias,
                                        alias,
                                        len > len2 ? len : len2)) {

                if (!show_any)
                    fprintf(stdout, "-*- You want to show '%s' session details.\n\n", alias);
                print_session_detail(i);
                show_any = 1;
                if(!show_all)
                    break;
            }
        }
    }

    if (!show_any)
        fprintf(stdout, "-!- There is no such a '%s' session existing.\n\n", alias);

}


void dbi_session_select (char* alias)
{
    int len = strlen(alias);
    if (!len) {
        fprintf(stdout, "-!- You must specify a valid session alias.\n\n");
        return;
    }
    if (!multi_session_mode) {
        fprintf(stdout, "-!- You must enable multi-session mode at first. Nothing is changed.\n\n");
        return;
    }

    int i, len2, found = 0;
    for (i = 0; i < CONN_SESSION_ENTRY_LIMIT; i++) {
        if(global_session_list[i].type) {
            len2 = strlen(global_session_list[i].alias);
            if(!strncasecmp(global_session_list[i].alias, alias,
                            len > len2 ? len : len2)) {
                fprintf(stdout, "-*- You have chosen '%s' as current active session.\n\n", alias);
                active_session_index = i;
                found = 1;
                break;
            }
        }
    }

    INTERFACE_DEBUG("Current active session index: %d", active_session_index);

    if (!found)
        fprintf(stdout, "-!- There is no such a '%s' valid session. Nothing is changed.\n\n", alias);
}


void dbi_preinit_all ()
{
    int i;
    for(i = 0; i < CONN_SESSION_ENTRY_LIMIT; i++)
        global_session_list[i].type = 0;
}


static void dbi_session_free(int index)
{
    if (!global_session_list[index].type)
        return;

#ifdef TDCLI_SUPPORT
    else if (global_session_list[index].type == TDCLI_SESSION) {
        free((td_session*)global_session_list[index].session);
    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[index].type == PQLIB_SESSION) {
        free((pq_session*)global_session_list[index].session);
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[index].type == MYSQL_SESSION) {
        free((mi_session*)global_session_list[index].session);
    }
#endif

    global_session_list[index].type = 0;
}


int dbi_init (int session_dbi)
{
    session_dbi = session_type(session_dbi);
    if (!(session_dbi == TDCLI_SESSION ||
          session_dbi == ODBC_SESSION  ||
          session_dbi == PQLIB_SESSION ||
          session_dbi == MYSQL_SESSION))
        return -1;

    int curr_index = active_session_index;
    if (multi_session_mode) {
        while (global_session_list[active_session_index].type) {

            active_session_index++;
            if(active_session_index + 1 == CONN_SESSION_ENTRY_LIMIT)
                active_session_index = 0;

            /* Cannot find a empty slot */
            if(curr_index == active_session_index) {
                fprintf(stdout, "-!- You can have at most %d sessions concurrently.\n",
                    CONN_SESSION_ENTRY_LIMIT);
                return -1;
            }
        }
    }
    else {
        if (global_session_list[active_session_index].type) {
            if (global_session_list[active_session_index].type == session_dbi)
                return 0;
            else
                dbi_session_free(active_session_index);
        }
    }

    global_session_list[active_session_index].type = session_dbi;
    snprintf(global_session_list[active_session_index].alias, CONN_SESSION_ALIAS_LENGTH, "conn%d", active_session_index);

    int ret;
    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef POSTGRESQL_SUPPORT
    else if (session_dbi == PQLIB_SESSION) {
        pq_session * conn_sess = (pq_session*)malloc(sizeof(pq_session));
        if (!conn_sess || pq_init(conn_sess))
            return -1;

        global_session_list[active_session_index].session = (void*)conn_sess;
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (session_dbi == MYSQL_SESSION) {
        mi_session * conn_sess = (mi_session*)malloc(sizeof(mi_session));
        if (!conn_sess || mi_init(conn_sess))
            return -1;

        global_session_list[active_session_index].session = (void*)conn_sess;
    }
#endif
#ifdef TDCLI_SUPPORT
    else if (session_dbi == TDCLI_SESSION) {
        td_session * conn_sess = (td_session*)malloc(sizeof(td_session));
        if(!conn_sess || tdcli_init(conn_sess) != EM_OK)
            return -1;

        global_session_list[active_session_index].session = (void*)conn_sess;
    }
#endif
#ifdef ODBC_SUPPORT
    else if (session_dbi == ODBC_SESSION) {
        odbc_session * conn_sess = (odbc_session*)malloc(sizeof(odbc_session));
        if (!conn_sess || odbc_init(conn_sess))
            return -1;

        global_session_list[active_session_index].session = (void*)conn_sess;
    }
#endif

    return 0;
}


int dbi_deinit_all ()
{
    int i;
    for (i = 0; i < CONN_SESSION_ENTRY_LIMIT; i++) {
        dbi_session_free(i);
    }

    return 0;
}


int dbi_connect (char* logon, int length)
{
    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef TDCLI_SUPPORT
    else if (global_session_list[active_session_index].type == TDCLI_SESSION) {
        if (tdcli_connect(global_session_list[active_session_index].session, logon) != EM_OK)
            return -1;
    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[active_session_index].type == PQLIB_SESSION) {
        if (pq_connect(global_session_list[active_session_index].session, logon, length))
            return -1;
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[active_session_index].type == MYSQL_SESSION) {
        if (mi_connect(global_session_list[active_session_index].session, logon, length))
            return -1;
    }
#endif
#ifdef ODBC_SUPPORT
    else if (global_session_list[active_session_index].type == ODBC_SESSION) {
        if (odbc_connect(global_session_list[active_session_index].session, logon, length))
            return -1;
    }
#endif

    return 0;
}


int dbi_execute (char *req)
{
    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef TDCLI_SUPPORT
    else if (global_session_list[active_session_index].type == TDCLI_SESSION) {
        if (tdcli_send_request(global_session_list[active_session_index].session, req) != EM_OK)
            return -1;
    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[active_session_index].type == PQLIB_SESSION) {
        if (pq_execute(global_session_list[active_session_index].session, req))
            return -1;
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[active_session_index].type == MYSQL_SESSION) {
        if (mi_execute(global_session_list[active_session_index].session, req))
            return -1;
    }
#endif
#ifdef ODBC_SUPPORT
    else if (global_session_list[active_session_index].type == ODBC_SESSION) {
        if (odbc_execute(global_session_list[active_session_index].session, req))
            return -1;
    }
#endif

    return 0;
}


int dbi_fetch()
{
    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef TDCLI_SUPPORT
    else if (global_session_list[active_session_index].type == TDCLI_SESSION) {
        if (tdcli_fetch_request(global_session_list[active_session_index].session) != EM_OK)
            return -1;
    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[active_session_index].type == PQLIB_SESSION) {
        if (pq_fetch(global_session_list[active_session_index].session))
            return -1;
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[active_session_index].type == MYSQL_SESSION) {
        if (mi_fetch(global_session_list[active_session_index].session))
            return -1;
    }
#endif
#ifdef ODBC_SUPPORT
    else if (global_session_list[active_session_index].type == ODBC_SESSION) {
        if (odbc_fetch(global_session_list[active_session_index].session))
            return -1;
    }
#endif

    return 0;
}

int dbi_finish()
{
    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef TDCLI_SUPPORT
    else if (global_session_list[active_session_index].type == TDCLI_SESSION) {
        if (tdcli_end_request(global_session_list[active_session_index].session) != EM_OK)
            return -1;
    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[active_session_index].type == PQLIB_SESSION) {
        if (pq_finish(global_session_list[active_session_index].session))
            return -1;
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[active_session_index].type == MYSQL_SESSION) {
        if (mi_finish(global_session_list[active_session_index].session))
            return -1;
    }
#endif
#ifdef ODBC_SUPPORT
    else if (global_session_list[active_session_index].type == ODBC_SESSION) {
        if (odbc_finish(global_session_list[active_session_index].session))
            return -1;
    }
#endif

    return 0;
}


static int dbi_end_one(int index)
{
    if ( 0 ) {
        /* dummy branch to ensure 'else if' is syntax-correct */
    }
#ifdef TDCLI_SUPPORT
    if (global_session_list[index].type == TDCLI_SESSION) {
        if (tdcli_end(global_session_list[index].session) != EM_OK)
            return -1;
    }
#endif
#ifdef POSTGRESQL_SUPPORT
    else if (global_session_list[index].type == PQLIB_SESSION) {
        if (pq_end(global_session_list[index].session))
            return -1;
    }
#endif
#ifdef MYSQL_SUPPORT
    else if (global_session_list[index].type == MYSQL_SESSION) {
        if (mi_end(global_session_list[index].session))
            return -1;
    }
#endif
#ifdef ODBC_SUPPORT
    else if (global_session_list[index].type == ODBC_SESSION) {
        if (odbc_end(global_session_list[index].session))
            return -1;
    }
#endif

    return 0;
}


int dbi_end()
{
    return dbi_end_one(active_session_index);
}


int dbi_end_all()
{
    int ret = 0, i, ret1;
    for (i = 0; i < CONN_SESSION_ENTRY_LIMIT; i++) {
        ret1 = dbi_end_one(i);
        if(ret1 &&!ret)
            ret = ret1;
    }

    return ret;
}


