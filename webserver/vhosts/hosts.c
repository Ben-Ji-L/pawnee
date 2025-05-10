#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>

#include "../file.h"
#include "hosts.h"
#include "../log/log.h"
#include "../config/config.h"
#include "../http/http_headers.h"

/**
 * read virtual host config files and search for the host wanted in the request
 * @param request the request to parse host header
 * @return the root for the virtual host
 */
char *get_vhost_root(http_request *request) {
    int bufferLength = 255;
    char buffer[bufferLength];
    char *host_content;

    char *host_header = get_header(&request->headers, "Host");
    
    // parsing host header
    if (host_header == NULL || strlen(host_header) == 0) {
        write_error(get_log_errors(), "missing host header");
        return NULL;
    }

    // Work on a copy of the host header to avoid modifying it destructively
    char host_copy[256];
    strncpy(host_copy, host_header, sizeof(host_copy));
    host_copy[sizeof(host_copy) - 1] = '\0';
    host_content = strtok(host_copy, ":");

    // Extract host content (before the ':')
    host_content = strtok(host_copy, ":");
    if (host_content == NULL) {
        write_error(get_log_errors(), "invalid host header");
        return NULL;
    }

    // flush buffer
    buffer[0] = '\0';

    // Get the path to the sites directory
    char *path = get_app_path();
    strncat(path, "/../config/sites", PATH_MAX - strlen(path) - 1);

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL) {
        write_error(get_log_errors(), "open sites directory error");
        free(path);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char vhost_path[PATH_MAX];
            snprintf(vhost_path, PATH_MAX, "%s/%s", path, entry->d_name);

            FILE *vhost_file = fopen(vhost_path, "r");
            if (vhost_file == NULL) {
                write_error(get_log_errors(), "open vhost config file error");
                closedir(dir);
                free(path);
                return NULL;
            }

            while (fgets(buffer, bufferLength, vhost_file)) {
                if (buffer[0] != '#') {
                    char *token = strtok(buffer, "=");
                    if (strcmp(token, "hostname") == 0) {
                        token = strtok(NULL, "=");
                        token = strtok(token, "\"");
                        if (strcmp(token, host_content) == 0) {
                            fgets(buffer, bufferLength, vhost_file);
                            token = strtok(buffer, "=");
                            if (strcmp(token, "root") == 0) {
                                token = strtok(NULL, "=");
                                char *result = strtok(token, "\"");
                                fclose(vhost_file);
                                closedir(dir);
                                free(path);
                                return strdup(result);
                            }
                        }
                    }
                }
            }
            fclose(vhost_file);
        }
    }
    closedir(dir);
    free(path);
    return NULL;
}