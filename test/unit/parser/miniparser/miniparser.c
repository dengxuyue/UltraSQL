#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "miniparser.h"
#include "parser/gram.c"
#include "parser/dots.c"
#include "parser/coarse.c"
#include "parser/miniparser.c"
#include "util/debug.c"

void expose_request (valid_request * req)
{
    if (req->validtext) {
        printf("\n--------------------------------------------------\n");
        printf("valid text:\n%s", req->validtext);
        printf("\n--------------------------------------------------\n");
    }
}

int init_suite1(void)
{
    init_trace();
    init_parser();
    return 0;
}

int clean_suite1(void)
{
    fini_parser();
    fini_trace();
    return 0;
}

#include "test_asserts_00x.c"

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
    if ((NULL == CU_add_test(pSuite, "test_parse_001()", test_parse_001)) ||
        (NULL == CU_add_test(pSuite, "test_parse_002()", test_parse_002)) ||
        (NULL == CU_add_test(pSuite, "test_parse_003()", test_parse_003)))
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
