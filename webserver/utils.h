# ifndef __UTILS_H__
# define __UTILS_H__

#include "http_parse.h"

char abs_exe_path[PATH_MAX];

char *fgets_or_exit(char *buffer, int size, FILE *stream);
void skip_headers(FILE * client);
void send_status(FILE *client, int code, const char *reason_phrase);
void send_response(FILE *client, int code, const char *reason_phrase, char *message_body, int size);
char *rewrite_target(char *target);
char *check_root(char *root);
FILE *check_and_open(const char *target, const char *document_root);
int get_file_size(int fd);
int copy(FILE *in, FILE *out);
char * get_mime_type(char *name);
void get_app_path(char *argv0);

#endif