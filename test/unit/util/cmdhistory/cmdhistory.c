#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "util/cmdhistory.c"

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_readline_0(void)
{
	int ret1 = init_history();
    CU_ASSERT(ret1 == 0);
	fini_history(0);
}

void test_readline_1(void)
{
	int ret1 = init_history();
    add_history("select * from dbc.dbcinfo;");
    CU_ASSERT(ret1 == 0);
	fini_history(1);
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
    if ((NULL == CU_add_test(pSuite, "test_readline_0()", test_readline_0)) ||
        (NULL == CU_add_test(pSuite, "test_readline_1()", test_readline_1)))
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
