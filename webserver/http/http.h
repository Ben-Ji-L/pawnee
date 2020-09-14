# ifndef __HTTP_H
#define __HTTP_H

void skip_headers(FILE *client);

void send_status(FILE *client, int code, const char *reason_phrase);

void send_response(FILE *client, int code, const char *reason_phrase, char *message_body, int size);

char *get_date_http_format(void);

char *rewrite_target(char *target);

#endif