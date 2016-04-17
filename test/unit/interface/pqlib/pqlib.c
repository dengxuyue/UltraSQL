#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "util/debug.c"
#include "util/helper.c"
#include "response/pager.c"
#include "interface/pqlib.c"

main(int argc, char **argv)
{
    int ret;
    pq_session session;

    ret = pq_init(&session);
    if(ret)
        printf("pq_init failed.\n");

    char conn_str[] = "host=localhost dbname=pagila";
    ret = pq_connect(&session, conn_str, strlen(conn_str));
    if(ret)
        printf("pq_connect failed.\n");
    pq_check_connection(&session);

    ret = pq_execute(&session, "select current_date;");
    if(ret)
        printf("pq_execute failed.\n");

    ret = pq_fetch(&session);
    if(ret)
        printf("pq_finish failed.\n");

    pq_finish(&session);
    pq_end(&session);
}
