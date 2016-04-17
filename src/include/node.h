#ifndef _NODE_H_
#define _NODE_H_

#include "config.h"

typedef struct logon_node_t {
    int  protocal;
    char datasource[LOGON_DATASOURCE_LENGTH];
    char username[LOGON_USERNAME_LENGTH];
    char password[LOGON_PASSWORD_LENGTH];
    char dbname[LOGON_DBNAME_LENGTH];
    char port[8]; /*You know it's a waste to define a macro*/
    char socket[LOGON_SOCKET_LENGTH];
} logon_node;

typedef struct set_node_t {
    int type;
    union {
        int  width;
        char delimiter[SET_DELIMITER_LENGTH];
        int  sqlsyntax; /* 0: off; 1: on */
        int  sqlcompliance;
        int  dbi;
    } value;
} set_node;

typedef struct session_node_t {
    int type;
    union {
        char alias[CONN_SESSION_ALIAS_LENGTH];
    } value;
} session_node;

typedef struct valid_request_t {
    int type;
    int subtype;
    char* validtext;
    char prealloc_text[1024];
    union {
        logon_node    logon;
        set_node      set;
        session_node  session;
    } node;
} valid_request;

#define DOT_DIRECTIVE 101
#define SQL_STATEMENT 102
#define END_DELIMITER 103

#define DTD_LOGON              1011
#define DTD_LOGOFF             1012
#define DTD_QUIT               1013

#define DTD_SET                1014
#define DTD_SET_WIDTH          10141
#define DTD_SET_DELIMITER      10142
#define DTD_SET_SQLSYNTAX      10143
#define DTD_SET_SQLCOMPLIANCE  10144

#define DTD_SESSION            1015
#define DTD_SESSION_SHOW       10151
#define DTD_SESSION_SELECT     10152

#define SQL_COMPLIANCE_DEFAULT     440
#define SQL_COMPLIANCE_ANSI        441
#define SQL_COMPLIANCE_TERADATA    442
#define SQL_COMPLIANCE_POSTGRESQL  443
#define SQL_COMPLIANCE_MYSQL       444

#define DBI_PROTOCAL_TDCLI         541
#define DBI_PROTOCAL_ODBC          542
#define DBI_PROTOCAL_POSTGRESQL    543
#define DBI_PROTOCAL_MYSQL         544


#endif /*_NODE_H_*/
