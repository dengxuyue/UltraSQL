#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "util/debug.c"
#include "util/helper.c"
#include "response/pager.c"
#include "interface/odbc.c"

int main(int argc, char **argv)
{
    odbc_session *sess = (odbc_session*)malloc(sizeof(odbc_session));
    if (!sess) {
        fprintf(stderr, "Cannot allocate odbc session.\n");
        return 1;
    }

    if (odbc_init(sess)) {
        fprintf(stderr, "Cannot initialize odbc session.\n");
        return 1;
    }

    if (odbc_connect(sess, "DSN=qh0004;", 12)) {
        fprintf(stderr, "Cannot connect to data source.\n");
        return 1;
    }
    
    if (odbc_execute(sess, "select * from dbc.dbcinfo;")) {
        fprintf(stderr, "Cannot execute a query.\n");
        return 1;
    }

    if (odbc_fetch(sess)) {
        fprintf(stderr, "Cannot fetch a result set.\n");
        return 1;
    }

    odbc_finish(sess);

    free(sess);

    return 0;
}
