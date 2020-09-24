#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CUnit/Basic.h"

#include "test_http.h"
#include "test.h"

void test_rewrite_target(void) {
    CU_ASSERT_STRING_EQUAL(rewrite_target("/"), "index.html");
    CU_ASSERT_STRING_EQUAL(rewrite_target("/?var=azee&varb=ertoro"), "index.html");
    CU_ASSERT_STRING_EQUAL(rewrite_target("/page.html"), "page.html");
}

void test_send_response(void) {
    send_response(temp_file, 200, "OK", "", 0);
}