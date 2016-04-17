#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "util/debug.c"
#include "util/helper.c"
#include "response/pager.c"
#include "interface/myclient.c"

main(int argc, char **argv)
{
    int ret;
    mi_session session;

    ret = mi_init(&session);
    if(ret)
        printf("mi_init failed.\n");

    char conn_str[] = "localhost;root;;mysql;;;";
    ret = mi_connect(&session, conn_str, strlen(conn_str));
    if(ret)
        printf("mi_connect failed.\n");

    ret = mi_execute(&session, "select current_date;");
    if(ret)
        printf("mi_execute failed.\n");

    ret = mi_fetch(&session);
    if(ret)
        printf("mi_finish failed.\n");

    mi_finish(&session);

    ret = mi_execute(&session, "select 1, 2;");
    if(ret)
        printf("mi_execute failed.\n");

    ret = mi_fetch(&session);
    if(ret)
        printf("mi_finish failed.\n");

    mi_finish(&session);


    mi_end(&session);
}
