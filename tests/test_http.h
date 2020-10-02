#ifndef TEST_HTTP_H
#define TEST_HTTP_H

#include "../webserver/http/http.h"
#include "../webserver/http/http_parse.h"

void test_rewrite_target(void);
void test_get_date_http_format(void);
void test_send_status(void);
void test_parse_http_request(void);
#endif
