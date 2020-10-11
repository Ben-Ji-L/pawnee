#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hosts.h"
#include "../log.h"

char *read_host_file(http_request *request);

char *get_vhost_root(http_request *request) {
    return read_host_file(request);
}

char *read_host_file(http_request *request) {
    FILE *host_file;
    int bufferLength = 255;
    char buffer[bufferLength];

    strtok(request->headers[0], ":");
    char *host_content = strtok(NULL, ":");
    host_content++;

    if ((host_file = fopen("config/hosts.cfg", "r")) == NULL) {
        write_error(get_log_errors(), "open hosts file error");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, bufferLength, host_file)) {
        // processing of the line
        if (buffer[0] != '#') {
            char *token = strtok(buffer, "=");

            // if the line is the port to listen
            if (strcmp(token, "hostname") == 0) {
                token = strtok(NULL, "=");
                token = strtok(token, "\"");
                if (strcmp(token, host_content) == 0) {
                    fgets(buffer, bufferLength, host_file);
                    char *token = strtok(buffer, "=");
                    if (strcmp(token, "root") == 0) {
                        token = strtok(NULL, "=");
                        char *result = strtok(token, "\"");
                        return result;
                    }
                }
            }
        }
    }
    return NULL;
}