#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

#include "http_headers.h"

void init_headers(http_headers *h) {
    h->count = 0;
    h->capacity = 8;
    h->headers = calloc(h->capacity, sizeof(http_header));
    if (!h->headers) {
        fprintf(stderr, "Erreur d'allocation mémoire dans init_headers\n");
        exit(EXIT_FAILURE);
    }
}

void add_header(http_headers *h, const char *name, const char *value) {
    if (h->count == h->capacity) {
        size_t new_capacity = h->capacity * 2;
        http_header *new_headers = realloc(h->headers, new_capacity * sizeof(http_header));
        if (!new_headers) {
            fprintf(stderr, "Erreur d'allocation mémoire dans add_header (realloc)\n");
            exit(EXIT_FAILURE);
        }
        h->headers = new_headers;
        h->capacity = new_capacity;
    }

    h->headers[h->count].name = strdup(name);
    h->headers[h->count].value = strdup(value);

    if (!h->headers[h->count].name || !h->headers[h->count].value) {
        fprintf(stderr, "Erreur d'allocation mémoire dans add_header (strdup)\n");
        exit(EXIT_FAILURE);
    }

    h->count++;
}

char *get_header(http_headers *headers, const char *name) {
    for (size_t i = 0; i < headers->count; ++i) {
        if (strcasecmp(headers->headers[i].name, name) == 0) {
            return headers->headers[i].value;
        }
    }
    return NULL;
}

void free_headers(http_headers *h) {
    if (!h || !h->headers) return;

    for (size_t i = 0; i < h->count; i++) {
        free(h->headers[i].name);
        free(h->headers[i].value);
    }
    free(h->headers);
    h->headers = NULL;
    h->count = 0;
    h->capacity = 0;
}
