# ifndef __HTTP_H
#define __HTTP_H

#include <stdio.h>

#include "../http/http_parse.h"

void send_status(int fd, http_request *request, int code, const char *reason_phrase);

void send_response(FILE *client, http_request *request, int code, const char *reason_phrase, char *message_body, int size);

char *get_date_http_format(void);

char *rewrite_target(char *target);

char *get_query_params(char *target);

int check_http_version(http_request *request);

#endif