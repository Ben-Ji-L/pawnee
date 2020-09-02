# ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>

FILE *check_and_open(const char *target, const char *document_root);
int get_file_size(int fd);
void copy(FILE *in, FILE *out);

#endif