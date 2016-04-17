void test_parse_001(void)
{
    valid_request req = {
        .type      = 0,
        .subtype   = 0,
        .validtext = 0,
    };
 
    int ret = parse("select 1", &req);
    expose_request(&req);
    CU_ASSERT(ret == 0);
}

void test_parse_002(void)
{
    valid_request req = {
        .type      = 0,
        .subtype   = 0,
        .validtext = 0,
    };
 
    reset_parser();

	int ret = parse("select begin(tt);", &req);
    expose_request(&req);
    CU_ASSERT(ret == 0);
}

void test_parse_003(void)
{
    valid_request req = {
        .type      = 0,
        .subtype   = 0,
        .validtext = 0,
    };
 
    reset_parser();

	int ret = parse("select 1, 2 from dbc.dbcinfo;", &req);
    expose_request(&req);
    CU_ASSERT(ret == 0);
}
