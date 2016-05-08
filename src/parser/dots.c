#include "miniparser.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

mini_dots_t set_dots_list[] = {
    {"width",          5, DTD_SET_WIDTH}
   ,{"delimiter",      9, DTD_SET_DELIMITER}
   ,{"sqlsyntax",      9, DTD_SET_SQLSYNTAX}
   ,{"sqlcompliance", 13, DTD_SET_SQLCOMPLIANCE}
   ,{"protocal",       8, DTD_SET_PROTOCAL}
   ,{"sidetitles",    10, DTD_SET_SIDETITLES}
};

mini_dots_t sql_compliance_list[] = {
    {"ansi",        4, SQL_COMPLIANCE_ANSI}
   ,{"teradata",    8, SQL_COMPLIANCE_TERADATA}
   ,{"postgresql", 10, SQL_COMPLIANCE_POSTGRESQL}
   ,{"mysql",       5, SQL_COMPLIANCE_MYSQL}
};

mini_dots_t session_list[] = {
    {"show",   4, DTD_SESSION_SHOW}
   ,{"select", 6, DTD_SESSION_SELECT}
};

mini_dots_t dbi_protocal_list[] = {
    {"cliv2",       5, DBI_PROTOCAL_TDCLI}
   ,{"odbc",        4, DBI_PROTOCAL_ODBC}
   ,{"pq",          2, DBI_PROTOCAL_POSTGRESQL}
   ,{"mysql",       5, DBI_PROTOCAL_MYSQL}
};


char internal_protocal[LOGON_PROTOCAL_LENGTH];


void set_parser_protocal (char* protoc)
{
    int len = strlen(protoc);
    if (len)
        strncpy(internal_protocal, protoc, len);
    internal_protocal[len] = '\0';
}


int parse_set (char* text, valid_request* req)
{
    req->node.set.type = 0;
    char* ptr = text;
    while(*ptr++ != '\0')
        /*leave out space*/
        if (!(*ptr == ' ' || *ptr == '\t'))
            break;

    int i, dots_list_size = sizeof(set_dots_list)/sizeof(set_dots_list[0]);
    char *pn;
    for(i = 0; i < dots_list_size; i++) {
        if(!strncasecmp(ptr, set_dots_list[i].name, set_dots_list[i].length)) {
            req->node.set.type = set_dots_list[i].value;
            pn = ptr + set_dots_list[i].length;
        }
    }
    if(!req->node.set.type)
        return 1;

    if(req->node.set.type == DTD_SET_WIDTH) {
        char num[SET_WIDTH_LENGTH];
        int nn = 0;
        while(*pn++ != '\0')
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;

        while(nn < SET_WIDTH_LENGTH - 1) {
            if (*pn >= '0' && *pn <= '9')
                num[nn++] = *pn++;
            else
                break;
        }
        num[nn + 1] = '\0';
        req->node.set.value.width = atoi(num);
        MINIPARSER_DEBUG("set-width: %d", req->node.set.value.width);
        return 0;
    }
    else if(req->node.set.type == DTD_SET_DELIMITER) {
        int len;
        while(*pn++ != '\0')
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;
        len = 0;
        while(len < SET_DELIMITER_LENGTH - 1) {
            if (*(pn + len) == ' '  || *(pn + len) == ';' ||
                *(pn + len) == '\t' || *(pn + len) == '\r' ||
                *(pn + len) == '\n' || *(pn + len) == '\0')
                break;
            len++;
        }
        if (len)
            strncpy(req->node.set.value.delimiter, pn, len);
        req->node.set.value.delimiter[len] = '\0';
        MINIPARSER_DEBUG("set-delimiter: %s", req->node.set.value.delimiter);
        return 0;
    }
    else if(req->node.set.type == DTD_SET_SQLSYNTAX) {
        int len;
        while(*pn++ != '\0')
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;
        len = 0;
        while(len < 3) { /* off/on */
            if (!((*(pn + len) >= 'a' && *(pn + len) <= 'z') ||
                  (*(pn + len) >= 'A' && *(pn + len) <= 'Z')))
                break;
            len++;
        }

        if (len < 2) {
            fprintf(stderr, "syntax error.\n");
            return 1;
        }
        else if (!strncasecmp(pn, "on", 2))
            req->node.set.value.sqlsyntax = 1;
        else if (!strncasecmp(pn, "off", len > 2 ? len : 2))
            req->node.set.value.sqlsyntax = 0;
        else {
            fprintf(stderr, "syntax error.\n");
            return 1;
        }

        MINIPARSER_DEBUG("set-sqlsyntax: %d", req->node.set.value.sqlsyntax);
        return 0;
    }
    else if(req->node.set.type == DTD_SET_SQLCOMPLIANCE) {
        int len;
        while(*pn++ != '\0')
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;
        len = 0;
        while(len < 10) { /* ansi/teradata/postgresql/mysql */
            if (!((*(pn + len) >= 'a' && *(pn + len) <= 'z') ||
                  (*(pn + len) >= 'A' && *(pn + len) <= 'Z')))
                break;
            len++;
        }

        req->node.set.value.sqlcompliance = 0;

        int k, llen, list_size = sizeof(sql_compliance_list)/sizeof(sql_compliance_list[0]);
        for(k = 0; k < list_size; k++) {
            llen = len;
            if (sql_compliance_list[k].length < llen)
                llen = sql_compliance_list[k].length;
            if(!strncasecmp(pn, sql_compliance_list[k].name, llen)) {
                req->node.set.value.sqlcompliance = sql_compliance_list[k].value;
            }
        }

        if (!req->node.set.value.sqlcompliance) {
            fprintf(stderr, "syntax error.\n");
            return 1;
        }

        MINIPARSER_DEBUG("set-sqlcompliance: %d", req->node.set.value.sqlcompliance);
        return 0;
    }
    else if(req->node.set.type == DTD_SET_PROTOCAL) {
        int len;
        while(*pn++ != '\0')
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;

        len = 0;
        while(len < 10) { /* cliv2/odbc/pq/mysql */
            if (!((*(pn + len) >= 'a' && *(pn + len) <= 'z') ||
                  (*(pn + len) >= 'A' && *(pn + len) <= 'Z') ||
                  (*(pn + len) >= '0' && *(pn + len) <= '9')))
                break;
            len++;
        }

        int k, llen = 0, found = 0;
        int list_size = sizeof(dbi_protocal_list)/sizeof(dbi_protocal_list[0]);
        for(k = 0; k < list_size; k++) {
            llen = len;
            if (dbi_protocal_list[k].length < llen)
                llen = dbi_protocal_list[k].length;
            if(!strncasecmp(pn, dbi_protocal_list[k].name, llen)) {
                if (llen) {
                    found = 1;
                    MINIPARSER_DEBUG("Get valid protocal '%s'", pn);
                    strncpy(req->node.set.value.protocal, pn, llen);
                }
            }
        }

        if (found)
            return 0;
    }
    else if(req->node.set.type == DTD_SET_SIDETITLES) {
        int len;
        while(*pn++ != '\0')
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;
        len = 0;
        while(len < 3) { /* off/on */
            if (!((*(pn + len) >= 'a' && *(pn + len) <= 'z') ||
                  (*(pn + len) >= 'A' && *(pn + len) <= 'Z')))
                break;
            len++;
        }

        if (len < 2) {
            fprintf(stderr, "syntax error.\n");
            return 1;
        }
        else if (!strncasecmp(pn, "on", 2))
            req->node.set.value.sidetitles = 1;
        else if (!strncasecmp(pn, "off", len > 2 ? len : 2))
            req->node.set.value.sidetitles = 0;
        else {
            fprintf(stderr, "syntax error.\n");
            return 1;
        }

        MINIPARSER_DEBUG("set-sidetitles: %d", req->node.set.value.sidetitles);
        return 0;
    }

    return 1;
}


/*
 * .logon [PROTOCAL://]HOSTNAME[:PORT]/USER[@DBNAME],PASSWORD
 */
int parse_logon (char* text, valid_request* req)
{
    int flag = 0;
    /* protocal : 1
     * hostname : 2
     * port     : 3
     * user     : 4
     * dbname   : 5
     * password : 6
     */

    char protocal[LOGON_PROTOCAL_LENGTH];
    strcpy(protocal, internal_protocal);

    req->node.logon.datasource[0] = '\0';
    req->node.logon.username[0]   = '\0';
    req->node.logon.password[0]   = '\0';
    req->node.logon.port[0]       = '\0';
    req->node.logon.dbname[0]     = '\0';
    req->node.logon.protocal      = 0;

    int len = strlen(text);
    int start, end, i = 0;

    for(; i < len; i++) {
        if (!(*(text + i) == ' ' || *(text + i) == '\t'))
            break;
    }
    start = i;
    /* keep i */

    /*
     * search for protocal/datasource
     */
    flag = 1;
    end = start;
    for(; i < len; i++) {
        if ((*(text + i) >= 'a' && *(text + i) <= 'z') ||
            (*(text + i) >= 'A' && *(text + i) <= 'Z') ||
            (*(text + i) >= '0' && *(text + i) <= '9') ||
            *(text + i) == '.' || *(text + i) == '_' || *(text + i) == '-')
            continue;
        else {
            end = i;
            break;
        }
    }

    for(; i < len; i++) {
        if (!(*(text + i) == ' ' || *(text + i) == '\t'))
            break;
    }

    if (*(text + i) == '/' )
        flag = 2; /*datasource*/
    else if (*(text + i) == ':' ) {
        if ( i + 1 < len && *(text + i + 1) == '/' &&
             i + 2 < len && *(text + i + 2) == '/' )
            flag = 1; /*procotal*/
        else if (i + 1 < len && *(text + i + 1) >= '0' && *(text + i + 1) <= '9' &&
                 i + 2 < len && *(text + i + 2) >= '0' && *(text + i + 2) <= '9' )
            flag = 2; /* datasource and port, e.g. hostname:99 */
        else  {
            fprintf(stderr, "syntax error: protocal/datasource is ill-formated.");
            return 1;
        }
    }

    if (flag == 1) {
        if(end - start + 1 < LOGON_PROTOCAL_LENGTH) {
            strncpy(protocal, text + start, end - start);
            protocal[end - start] = '\0';
            MINIPARSER_DEBUG("protocal: %s", protocal);
        }
        else {
            fprintf(stderr, "syntax error: protocal is too long: %d.", end - start + 1);
            return 1;
        }

        /* continue to serach for datasource */
        i += 3;
        for(; i < len; i++)
            if (!(*(text + i) == ' ' || *(text + i) == '\t'))
                break;
        start = i;
        for(; i < len; i++) {
            if ((*(text + i) >= 'a' && *(text + i) <= 'z') ||
                (*(text + i) >= 'A' && *(text + i) <= 'Z') ||
                (*(text + i) >= '0' && *(text + i) <= '9') ||
                *(text + i) == '.' || *(text + i) == '_' || *(text + i) == '-')
                continue;
            else {
                end = i;
                break;
            }
        }
        if (*(text + i) == ':' || *(text + i) == '/' ) {
            flag = 2;
        }
        else {
            fprintf(stderr, "syntax error: datasource is necessary.");
            return 1;
        }
    }

    int k, list_size = sizeof(dbi_protocal_list)/sizeof(dbi_protocal_list[0]);
    for(k = 0; k < list_size; k++ ) {
        if(!strncasecmp(protocal, dbi_protocal_list[k].name, dbi_protocal_list[k].length)) {
            req->node.logon.protocal = dbi_protocal_list[k].value;
            MINIPARSER_DEBUG("Get valid protocal %s", protocal);
            MINIPARSER_DEBUG("Dot protocal type is %d", req->node.logon.protocal);
            break;
        }
    }
    if (!req->node.logon.protocal){
        fprintf(stderr, "syntax error: protocal is invalid: %s.", protocal);
        return 1;
    }

    if (flag == 2) {
        if(end - start + 1 < LOGON_DATASOURCE_LENGTH) {
            strncpy(req->node.logon.datasource, text + start, end - start);
            *(req->node.logon.datasource + end - start) = '\0';
            MINIPARSER_DEBUG("datasource: %s", req->node.logon.datasource);
        }
        else  {
            fprintf(stderr, "syntax error: datasource is too long: %d.", end - start + 1);
            return 1;
        }
    }
    else {
        fprintf(stderr, "syntax error: datasource is necessary.");
        return 1;
    }

    /* search for port */
    if (*(text + i) == ':') {
        flag = 3;
        i++;
        for(; i < len; i++)
            if (!(*(text + i) == ' ' || *(text + i) == '\t'))
                break;
        start = i;
        for (; i < len; i++) {
            if (!(*(text + i) >= '0' && *(text + i) <= '9'))
                break;
        }
        end = i;

        if(end - start + 1 < 8) {
            strncpy(req->node.logon.port, text + start, end - start);
            *(req->node.logon.port + end - start) = '\0';
            MINIPARSER_DEBUG("port: %s", req->node.logon.port);
        }
        else {
            fprintf(stderr, "syntax error: port is too long: %d.", end - start + 1);
            return 1;
        }
    }

    /* assertation! */
    if (*(text + i) != '/') {
        fprintf(stderr, "syntax error: user is necessary.");
        return 1;
    }

    /* search for user */
    i++;
    for(; i < len; i++)
        if (!(*(text + i) == ' ' || *(text + i) == '\t'))
            break;
    start = i;
    for(; i < len; i++) {
        if ((*(text + i) >= 'a' && *(text + i) <= 'z') ||
            (*(text + i) >= 'A' && *(text + i) <= 'Z') ||
            (*(text + i) >= '0' && *(text + i) <= '9') ||
            *(text + i) == '_' )
            continue;
        else {
            end = i;
            break;
        }
    }
    if (len == i)
        end = i;
    if (end > start)
        flag = 4;

    if (flag == 4) {
        if (end - start + 1 < LOGON_USERNAME_LENGTH) {
            strncpy(req->node.logon.username, text + start, end - start);
            *(req->node.logon.username + end - start) = '\0';
            MINIPARSER_DEBUG("username: %s", req->node.logon.username);
        }
        else  {
            fprintf(stderr, "syntax error: user name is too long: %d.\n", end - start + 1);
            return 1;
        }
    }
    else {
        fprintf(stderr, "syntax error: user name is necessary.\n");
        return 1;
    }

    /* search for dbname */
    if (i < len && *(text +i) == '@') {
        flag = 5;
        i++;
        start = i;
        for(; i < len; i++) {
            if ((*(text + i) >= 'a' && *(text + i) <= 'z') ||
                (*(text + i) >= 'A' && *(text + i) <= 'Z') ||
                (*(text + i) >= '0' && *(text + i) <= '9') ||
                *(text + i) == '_' )
                continue;
            else {
                end = i;
                break;
            }
        }
        if (len == i)
            end = i;

        if(end - start + 1 < LOGON_DBNAME_LENGTH) {
            strncpy(req->node.logon.dbname, text + start, end - start);
            *(req->node.logon.dbname + end - start) = '\0';
            MINIPARSER_DEBUG("dbname: %s", req->node.logon.dbname);
        }
        else {
            fprintf(stderr, "syntax error: dbname is too long: %d.", end - start + 1);
            return 1;
        }
    }


    return 0;
}


int parse_session (char* text, valid_request* req)
{
    req->node.session.type = 0;
    char* ptr = text;
    while(*ptr++ != '\0')
        /*leave out space*/
        if (!(*ptr == ' ' || *ptr == '\t'))
            break;

    int i, list_size = sizeof(session_list)/sizeof(session_list[0]);
    char *pn;
    for(i = 0; i < list_size; i++) {
        if(!strncasecmp(ptr, session_list[i].name, session_list[i].length)) {
            req->node.session.type = session_list[i].value;
            pn = ptr + session_list[i].length;
        }
    }
    if(!req->node.session.type)
        return 1;

    if (req->node.session.type == DTD_SESSION_SHOW ||
        req->node.session.type == DTD_SESSION_SELECT) {
        req->node.session.value.alias[0] = '\0';

        while(*pn != '\0')  {
            /*leave out space*/
            if (!(*pn == ' ' || *pn == '\t'))
                break;
            pn++;
        }

        int len = 0;
        while( *(pn + len) != '\0' &&
               ((*(pn + len) >= '0' && *(pn + len) <= '9') ||
                (*(pn + len) >= 'a' && *(pn + len) <= 'z') ||
                (*(pn + len) >= 'A' && *(pn + len) <= 'Z')))
            len++;
        if (len)
            len++;
        if (len > CONN_SESSION_ALIAS_LENGTH - 1)
            len = CONN_SESSION_ALIAS_LENGTH - 1;
        if (len) {
            snprintf(req->node.session.value.alias, len, pn);
            req->node.session.value.alias[len + 1] = '\0';
        }

        if (req->node.session.type == DTD_SESSION_SHOW) {
            MINIPARSER_DEBUG("session-show: %s", req->node.session.value.alias);
        }
        else if (req->node.session.type == DTD_SESSION_SELECT) {
            MINIPARSER_DEBUG("session-select: %s", req->node.session.value.alias);
        }

        return 0;
    }

    return 0;
}
