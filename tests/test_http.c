#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "CUnit/Basic.h"

#include "test_http.h"
#include "test.h"

void test_rewrite_target(void) {
    CU_ASSERT_STRING_EQUAL(rewrite_target("/"), "index.html");
    CU_ASSERT_STRING_EQUAL(rewrite_target("/?var=azee&varb=ertoro"), "index.html");
    CU_ASSERT_STRING_EQUAL(rewrite_target("/page.html"), "page.html");
}

void test_get_date_http_format(void) {
    time_t rawtime;
    struct tm *timeinfo;
    char *date = malloc(100);

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(date, 40, "%a, %d %b %G %X %Z", timeinfo);
    CU_ASSERT_PTR_NOT_NULL(date);
    CU_ASSERT_STRING_EQUAL(get_date_http_format(), date);
}

void test_send_status(void) {
    const size_t line_size = 300;
    char* line = malloc(line_size);

    send_status(get_temp_file(), 200, "OK");
    rewind(get_temp_file());

    fgets(line, line_size, get_temp_file());
    CU_ASSERT_STRING_EQUAL(line, "HTTP/1.1 200 OK\r\n");
    free(line);
}

void test_parse_http_request(void) {
    http_request request;
    parse_http_request("GET /index.html HTTP/1.1\r\n", &request);
    CU_ASSERT_EQUAL(request.http_major, 1);
    CU_ASSERT_EQUAL(request.http_minor, 1);
    CU_ASSERT_EQUAL(request.method, HTTP_GET);

    parse_http_request("PUT / HTTP/1.1\r\n", &request);
    CU_ASSERT_EQUAL(request.method, HTTP_UNSUPPORTED);

    parse_http_request("ZLA / HTTP/1.1\r\n", &request);
    CU_ASSERT_EQUAL(request.method, HTTP_UNSUPPORTED);

    parse_http_request("GET / HTTP/42.1\r\n", &request);
    CU_ASSERT_EQUAL(request.http_major, 1);
}