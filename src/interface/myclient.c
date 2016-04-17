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

    MYSQL_RES * res = mysql_use_result(sess->connection);
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
    unsigned int num_fields, i;
    unsigned long *lengths;

    num_fields = mysql_num_fields(sess->portal);
    fields     = mysql_fetch_fields(sess->portal);
    if (!fields) {
        fprintf(stdout, "-!- Errors in retrieving the column names of the query result..\n");
    }
    for(i = 0; i < num_fields; i++) {
        printf("%.*s", fields[i].name_length, fields[i].name); 
    }
    printf("\n");

    while ((row = mysql_fetch_row(sess->portal)))
    {
        lengths  = mysql_fetch_lengths(sess->portal);
        for(i = 0; i < num_fields; i++) {
            printf("[%.*s] ", (int) lengths[i],
                row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

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

