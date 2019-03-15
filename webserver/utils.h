# ifndef __UTILS_H__
# define __UTILS_H__

#include "http_parse.h"

char *fgets_or_exit(char *buffer, int size, FILE *stream);
void skip_headers(FILE * client);
void send_status(FILE *client, int code, const char *reason_phrase);
void send_response(FILE *client, int code, const char *reason_phrase,
const char *message_body);
char *rewrite_target(char *target);

#endif