#include "config.h"
#include "myclient.h"
#include "pager.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static int interrupt_mysql_fetching = 0;
void mysql_inter_fetch (int signo)
{
    interrupt_mysql_fetching = 1;
}


static int sidetitles_mysql = 0;
void set_my_sidetitles(int on_off)
{
    sidetitles_mysql = on_off ? 1 : 0;
}


inline void mysql_write_error(MYSQL *conn)
{
    fprintf(stdout, "-!- Error code   : %u\n", mysql_errno(conn));
    fprintf(stdout, "-!- Error message: %s\n", mysql_error(conn));
}


int mi_init (mi_session * sess)
{
    if (!sess)
        return -1;

    MYSQL *conn;
    conn = mysql_init(NULL);

    if (!conn) {
        fprintf(stdout, "-!- Error in initialization of MySQL connection.\n");
        return -1;
    }

    sess->connection = conn;
    sess->portal     = 0;

    return 0;
}


int mi_connect (mi_session* sess, char* conninfo, int infolen)
{
    if (!sess)
        return -1;

    INTERFACE_DEBUG("Connection info: %s", conninfo);

    char datasource[LOGON_DATASOURCE_LENGTH];
    char username[LOGON_USERNAME_LENGTH];
    char password[LOGON_PASSWORD_LENGTH];
    char dbname[LOGON_DBNAME_LENGTH];
    char port[8];
    char socket[LOGON_SOCKET_LENGTH];

    /* datasource */
    int start, i;
    int len   = strlen(conninfo);
    i         = 0;
    start     = 0;
    while(i < len) {
        if (conninfo[i] == ';')
            break;
        i++;
    }
    snprintf(datasource, i - start + 1, conninfo + start);
    datasource[i - start + 1] = '\0';
    INTERFACE_DEBUG("Logon datasource: %s", datasource);

    /* username */
    i++;
    start = i;
    while(i < len) {
        if (conninfo[i] == ';')
            break;
        i++;
    }
    snprintf(username, i - start + 1, conninfo + start);
    username[i - start + 1] = '\0';
    INTERFACE_DEBUG("Logon username: %s", username);

    /* password */
    char * eff_passwd = 0;
    i++;
    start = i;
    while(i < len) {
        if (conninfo[i] == ';')
            break;
        i++;
    }
    if (i > start) {
        snprintf(password, i - start + 1, conninfo + start);
        password[i - start + 1] = '\0';
        INTERFACE_DEBUG("Logon password: %s", password);
        eff_passwd = password;
    }

    /* dbname */
    char * eff_db = 0;
    i++;
    start = i;
    while(i < len) {
        if (conninfo[i] == ';')
            break;
        i++;
    }
    if (i > start) {
        snprintf(dbname, i - start + 1, conninfo + start);
        dbname[i - start + 1] = '\0';
        INTERFACE_DEBUG("Logon dbname: %s", dbname);
        eff_db = dbname;
    }

    /* port */
    int eff_port = 0;
    i++;
    start = i;
    while(i < len) {
        if (conninfo[i] == ';')
            break;
        i++;
    }
    if (i > start) {
        snprintf(port, i - start + 1, conninfo + start);
        port[i - start + 1] = '\0';
        INTERFACE_DEBUG("Logon port: %s", port);
        eff_port = atoi(port);
    }

    sess->portal = 0;
    if (mysql_real_connect(sess->connection, datasource, username,
                           eff_passwd, eff_db, eff_port, NULL, 0))
        return 0;

    fprintf(stdout, "-!- Error when connecting to MySQL server.\n");
    mysql_write_error(sess->connection);
    return -1;
}


int mi_execute (mi_session* sess, char* req)
{
    if (!req || !sess || !sess->connection)
        return -1;

    sess->portal = 0;
    if (mysql_query(sess->connection, req)) {
        fprintf(stdout, "-!- Error when executiong the query just submitted..\n");
        mysql_write_error(sess->connection);
        return -1;
    }

    /*
     * Gradually retrieve result set
    MYSQL_RES * res = mysql_use_result(sess->connection);
    */
    MYSQL_RES * res = mysql_store_result(sess->connection);
    if(!res) {
        fprintf(stdout, "-!- Error when initializing the query result..\n");
        mysql_write_error(sess->connection);
        return -1;
    }
    sess->portal = res;

    return 0;
}


int mi_fetch (mi_session* sess)
{
    if (!sess || !sess->connection || !sess->portal)
        return -1;

    MYSQL_FIELD *fields;
    MYSQL_ROW   row;
    MYSQL_RES   res;

    unsigned int num_rows = 0, num_fields, i, j;
    unsigned long *lengths;
    int len;

    num_rows   = mysql_num_rows(sess->portal);
    num_fields = mysql_num_fields(sess->portal);
    fields     = mysql_fetch_fields(sess->portal);
    if (!fields) {
        fprintf(stdout, "-!- Errors in retrieving the column names of the query result..\n");
        return 1;
    }
    fprintf(stdout, "-*- Query has been complete. %d rows %d columns returned.\n\n",
                num_rows, num_fields);

    FILE* fout = stdout;
    if (num_rows > DEFAULT_SCREEN_HEIGHT) {
        open_resp_pager();
        if(internal_pager) {
            fout = internal_pager;
            signal_resp_pager(mysql_inter_fetch);
        }
    }

    /* sidetitles = on */
    if(!sidetitles_mysql) {

        /*
         * print result set header
         */
        for(i = 0; i < num_fields; i++) {
            if (i) fprintf(fout, "|");
            len = fprintf(fout, "%s", fields[i].name);
            for(j = len; j < MAX(fields[i].name_length, fields[i].max_length);  j++)
                fprintf(fout, " ");
        }
        fprintf(fout, "\n");
        for(i = 0; i < num_fields; i++) {
            if (i) fprintf (fout, "+");
            for(j = 0; j < MAX(fields[i].name_length, fields[i].max_length);  j++)
                fprintf(fout, "-");
        }
        fprintf(fout, "\n");
    }

    /* for sidetiles */
    signed int max_title_length = 0;
    int max_field_length;

    while ((row = mysql_fetch_row(sess->portal)))
    {
       /* it is not necesary!
        * lengths  = mysql_fetch_lengths(sess->portal);
        */

        if (sidetitles_mysql) {
            /* sidetitles = on */

            if (max_title_length < 1)
                for(i = 0; i < num_fields; i++)
                    if (max_title_length < fields[i].name_length)
                        max_title_length = fields[i].name_length;

            max_field_length = -1;
            for(i = 0; i < num_fields; i++) {
                len = fprintf(fout, fields[i].name);
                for(j = len; j < max_title_length; j++)
                    fprintf(fout, " ");

                fprintf(fout, "|");
                len = fprintf(fout, "%s", (row[i] ? row[i] : "NULL"));
                if (len > max_field_length)
                    len = max_field_length;
                fprintf(fout, "\n");
            }

            /* separator */
            for(j = 0; j < max_title_length; j++)
                fprintf(fout, "-");
            fprintf(fout, "+");
            for(j = max_title_length + 1;
                j < MAX(max_title_length + 1 + max_field_length, 75);
                j++)
                fprintf(fout, "-");
        }
        else {
            /* sidetitles = off */
            for(i = 0; i < num_fields; i++) {
                if (i) fprintf (fout, "|");
                len = fprintf(fout, "%s", (row[i] ? row[i] : "NULL"));
                for(j = len; j < MAX(fields[i].name_length, fields[i].max_length);  j++)
                    fprintf(fout, " ");
            }
        }

        fprintf(fout, "\n");
        if (interrupt_mysql_fetching)
            break;
    }

    signal_resp_pager(SIG_IGN);
    close_resp_pager();

    return 0;
}


int mi_finish (mi_session* sess)
{
    if (sess && sess->portal) {
        mysql_free_result(sess->portal);
        sess->portal = 0;
    }

    return 0;
}


int mi_end (mi_session* sess)
{
    if (sess) {
        mysql_close(sess->connection);
        sess->connection = 0;
    }

    return 0;
}

