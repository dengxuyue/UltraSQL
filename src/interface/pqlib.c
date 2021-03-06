/*
 * summary: communication module with PostgreSQL
 *
 * Status:
 *     No big feature is underlying development
 */
#include "config.h"
#include "pqlib.h"
#include "pager.h"
#include "pqtuples.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static int interrupt_pqlib_fetching = 0;
void pqlib_inter_fetch (int signo)
{
    interrupt_pqlib_fetching = 1;
}

void set_pq_sidetitles(int on_off)
{
    set_pq_sidetitles_impl(on_off);
}


int pq_init (pq_session * sess)
{
    if (!sess)
        return -1;

    sess->def_pg_port[0] = '\0';
    sess->connection = 0;
    sess->portal     = 0;
    char* port;
    if (port = getenv("PGPORT")) {
        strncpy(sess->def_pg_port, port, 8);
    }

    return 0;
}


int pq_connect (pq_session* sess, char* conninfo, int infolen)
{
    if (!sess)
        return -1;

    /* Check for 'port' */
    if (strlen(sess->def_pg_port)) {
        int length = strlen(conninfo);
        int found  = 0;
        int i = 0;
        while (i < length - 4) {
            if ((conninfo[i + 0] == 'p' &&
                 conninfo[i + 1] == 'o' &&
                 conninfo[i + 2] == 'r' &&
                 conninfo[i + 3] == 't')
                &&
                (i == 0 || conninfo[i - 1] == ' ')) {
                found = 1;
                break;
            }
            i++;
        }
        if(found){
            while(i < length && conninfo[i] == ' ')
                i++;
            if (conninfo[i] == '=')  /* there is no "port =" */
                found = 2;
        }

        if (found < 2)
            snprintf(conninfo + length, infolen - length, " port=%s", sess->def_pg_port);
    }

    PGconn* conn = PQconnectdb(conninfo);
    if(!conn)
        return -1;
    sess->connection = conn;

    int status_OK = 0;
    int retries   = 0;
    while (retries < 12 && !status_OK) {
        if (retries) {
        /* waiting for 5 seconds if connection is neither bad nor OK */
            if(DEBUG_ON & 0xF0)
                printf("Waiting for connection is made ...\n");
            sleep(5);
        }

        switch (PQstatus(conn)) {
        case CONNECTION_OK:
            status_OK = 1;
            break;
        case CONNECTION_BAD:
            status_OK = -1;
            break;
        default:
            break;
        }
    }

    if (status_OK == 1)
        return 0;

    return 1;
}


int pq_check_connection (pq_session* sess)
{
    switch(PQstatus(sess->connection))
    {
    case CONNECTION_STARTED:
        if(DEBUG_ON & 0xF0)
            printf("Started to connect ...\n");
        break;

    case CONNECTION_MADE:
        if(DEBUG_ON & 0xF0)
            printf("Connection is made ...\n");
        break;

    case CONNECTION_AWAITING_RESPONSE:
        if(DEBUG_ON & 0xF0)
            printf("Waiting for response ...\n");
        break;

    case CONNECTION_AUTH_OK:
        if(DEBUG_ON & 0xF0)
            printf("Authentication is ready ...\n");
        break;

    case CONNECTION_SSL_STARTUP:
        if(DEBUG_ON & 0xF0)
            printf("Starting up SSL encryption ...\n");
        break;

    case CONNECTION_SETENV:
        if(DEBUG_ON & 0xF0)
            printf("Negotiating env parameters ...\n");
        break;

    case CONNECTION_OK:
        if(DEBUG_ON & 0xF0)
            printf("Connection is OK ...\n");
        return 0;

    case CONNECTION_BAD:
        if(DEBUG_ON & 0xF0)
            printf("Connection is bad ...\n");
        return -1;

    default:
        if(DEBUG_ON & 0xF0)
            printf("Connecting ...\n");
        break;
    }

    return 1;
}


int pq_execute (pq_session* sess, char* req)
{
    if (!req || !sess || !sess->connection)
        return -1;

    if (pq_check_connection(sess)) {
        fprintf(stdout, "-!- The connection is not ready.\n");
        return -1;
    }

    PGresult* res = PQexec(sess->connection, req);
    if (!res)
        return -1;

    sess->portal = res;

    return 0;
}


int pq_fetch (pq_session* sess)
{
    if (!sess || !sess->portal)
        return -1;

    PGresult * result = sess->portal;

    int rows = PQntuples(result);
    int cols = PQnfields(result);

    int j, i, elt_len;
    int* field_length = 0;

    FILE * fout = stdout;
    if (cols && rows) {
        /* get length list for all fields */
        field_length = (int*) malloc(sizeof(int) * cols);
        if (!field_length)
            return -1;

        for (i = 0; i < cols; i++) {
            field_length[i] = 0;
            for (j = 0; j < rows; j++) {
                elt_len = PQgetlength(result, j, i);
                if (elt_len > field_length[i])
                    field_length[i] = elt_len;
            }
        }
    }

    char* col_title;
    int   len_title;
    for (i = 0; i < cols; i++) {
        if(col_title =  PQfname(result, i)) {
            len_title = strlen(col_title);
            if (len_title > field_length[i])
                field_length[i] = len_title;

            append_title(field_length[i], col_title);
        }
    }

    printf("-*- Query has been complete. %d rows %d columns returned.\n\n",
                rows, cols);

    flush_title();
    if (rows > DEFAULT_SCREEN_HEIGHT) {
        open_resp_pager();
        if(internal_pager) {
            signal_resp_pager(pqlib_inter_fetch);
        }
    }

    char* elt_val;
    for (j = 0; j < rows; j++) {
        if (interrupt_pqlib_fetching)
            break;
        for (i = 0; i < cols; i++) {
            elt_val = PQgetvalue(result, j, i);
            elt_len = PQgetlength(result, j, i);
            append_tuple(MAX(elt_len, field_length[i]), elt_val);
        }
        flush_tuple();
    }

    flush_end();
    if (!field_length)
        free(field_length);
    signal_resp_pager(SIG_IGN);
    close_resp_pager();

    printf("\n\n");

    return 0;
}


int pq_finish (pq_session* sess)
{
    if (sess && sess->portal)
        PQclear(sess->portal);

    return 0;
}


int pq_end (pq_session* sess)
{
    if (sess)
        PQfinish(sess->connection);

    return 0;
}

