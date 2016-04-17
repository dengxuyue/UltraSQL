#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "util/debug.c"
#include "util/helper.c"
#include "response/parcels.c"
#include "response/pager.c"
#include "interface/tdcli.c"

main(int argc, char **argv)
{
    td_session session;
    unsigned int MaxSegSize;
    char str1[] = "replace procedure tmp(IN p1 integer,IN p2 integer, OUT p3 integer)\n begin\n set p3=p1*p2;\n end;\n";
    char str2[] = "CALL tmp(2,3,p3);";
    char str3[] = "RENAME PROCEDURE tmp TO tmp1;";
    char str4[] = "SHOW PROCEDURE tmp1;";
    char str5[] = "HELP PROCEDURE tmp1 attributes;";
    char str6[] = "HELP 'spl WHILE' ;";
    char str7[] = "DROP PROCEDURE tmp1;";
    char str8[] = "select * from test.t01;";
    char psDefaultLogon[] = "localhost/test,test";
    char* psLogon = psDefaultLogon;
    if (argc >= 2)
        psLogon = argv[1];
    if (psLogon == NULL)
        psLogon = psDefaultLogon;
    if (!strncmp(psLogon, "-h", 2))
    {
        printf ("clisamp logonstring(dbcname/username,password)\n");
        exit(1);
    }

    printf("\nCLIv2 version is %s\n", COPCLIVersion);
    printf("MTDP version is %s\n", COPMTDPVersion);
    printf("MOSIOS version is %s\n", COPMOSIosVersion);
    printf("MOSIDEP version is %s\n", COPMOSIDEPVersion);
    printf("OSERR version is %s\n\n", OSERRVersion);

    tdcli_init(&session);
    init_resp_buffer();

    tdcli_connect(&session, psLogon);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_set_dbcareax(&session);

    tdcli_send_request(&session, str1);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);
    tdcli_clear_dbcareax(&session);

    /****************************************/

    tdcli_send_request(&session, str2);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_send_request(&session, str3);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_send_request(&session, str4);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_send_request(&session, str5);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_send_request(&session, str6);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_send_request(&session, str7);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    /****************************************/

    printf("-----------------Last request-----------------------\n");
    tdcli_send_request(&session, str8);
    tdcli_fetch_request(&session);
    tdcli_end_request(&session);

    tdcli_end(&session);
    fini_resp_buffer();
}
