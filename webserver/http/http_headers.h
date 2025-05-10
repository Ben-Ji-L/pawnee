#ifndef __HTTP_HEADERS__
#define __HTTP_HEADERS__

#include <stddef.h>

typedef struct {
    char *name;
    char *value;
} http_header;

typedef struct {
    http_header *headers;
    size_t count;
    size_t capacity;
} http_headers;

void init_headers(http_headers *h);

void add_header(http_headers *h, const char *name, const char *value);

char *get_header(http_headers *h, const char *name);

void free_headers(http_headers *h);

#endif