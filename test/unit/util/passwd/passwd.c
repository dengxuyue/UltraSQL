#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "util/profile.c"
#include "util/passwd.c"
#include "util/setting.c"

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_passwd_0(void)
{
    init_profile();
    int i;
    printf("\n");
    for(i = 0; i <= dbs_user_passwds; i++) {
        printf("dbs: %s; user: %s; passwd: %s.\n", 
            dbs_user_passwd[i].datasource,
            dbs_user_passwd[i].username,
            dbs_user_passwd[i].password);

    }
    printf("\n");
    CU_ASSERT(i > 0);

    fini_profile();
}

void test_passwd_1(void)
{
    init_profile();
	char* pw = get_password("localhost", "dbc");
    printf("\npassword: %s.\n", pw);
    fini_profile();

    CU_ASSERT(pw);
}

int main()
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
    if ((NULL == CU_add_test(pSuite, "test_passwd_0()", test_passwd_0)) ||
        (NULL == CU_add_test(pSuite, "test_passwd_1()", test_passwd_1)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
