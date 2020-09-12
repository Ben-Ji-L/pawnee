# ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>

FILE *check_and_open(const char *target, const char *document_root);

int get_file_size(int fd);

void copy(FILE *in, FILE *out);

char *fgets_or_exit(char *buffer, int size, FILE *stream);

char *check_root(char *root);

char *get_mime_type(char *name);

char *get_app_path(char *argv0);

#endif