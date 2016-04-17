#ifndef _ODBC_H_
#define _ODBC_H_

#include <iconv.h>
#include <sql.h>
#include <sqlext.h>
#include "config.h"

typedef struct odbc_session_t {
    SQLHENV    env;
    SQLHDBC    dbc;
    SQLHSTMT   stmt;
    int        status;
    /*
     * status (b2b1b0):
     *   b0: env
     *   b1: dbc
     *   b2: stmt
     */
} odbc_session;


int odbc_init(odbc_session *sess);
int odbc_connect(odbc_session *sess, char* logon, int length);
int odbc_execute(odbc_session *sess, char* req);
int odbc_fetch(odbc_session *sess);
int odbc_finish(odbc_session *sess);

#endif /* _ODBC_H_ */
