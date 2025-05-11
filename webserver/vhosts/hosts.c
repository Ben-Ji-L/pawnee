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

vhost_config *get_vhost_config(http_request *request) {
    char buffer[256];
    char *host_header = get_header(&request->headers, "Host");

    if (!host_header || strlen(host_header) == 0) {
        write_error(get_log_errors(), "missing or empty Host header");
        return NULL;
    }

    // Extraire le host sans le port
    char host_clean[256];
    strncpy(host_clean, host_header, sizeof(host_clean));
    host_clean[sizeof(host_clean) - 1] = '\0';
    strtok(host_clean, ":");

    // Obtenir chemin vers le dossier des vhosts
    char *sites_dir = get_app_path();
    strncat(sites_dir, "/../config/sites", PATH_MAX - strlen(sites_dir) - 1);

    DIR *dir = opendir(sites_dir);
    if (!dir) {
        write_error(get_log_errors(), "cannot open sites directory");
        free(sites_dir);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG) continue;

        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/%s", sites_dir, entry->d_name);
        FILE *file = fopen(filepath, "r");
        if (!file) continue;

        char *hostname = NULL, *root = NULL, *log_dir = NULL;

        while (fgets(buffer, sizeof(buffer), file)) {
            if (buffer[0] == '#') continue;

            char *key = strtok(buffer, "=");
            char *value = strtok(NULL, "\"");

            if (!key || !value) continue;

            if (strcmp(key, "hostname") == 0) {
                hostname = strdup(value);
            } else if (strcmp(key, "root") == 0) {
                root = strdup(value);
            } else if (strcmp(key, "log_dir") == 0) {
                log_dir = strdup(value);
            }
        }

        fclose(file);

        if (hostname && strcmp(hostname, host_clean) == 0) {
            closedir(dir);
            free(sites_dir);

            vhost_config *config = malloc(sizeof(vhost_config));
            config->hostname = strdup(hostname);
            config->root = strdup(root);
            config->log_dir = strdup(log_dir);
            return config;
        }

        free(hostname);
        free(root);
        free(log_dir);
    }

    closedir(dir);
    free(sites_dir);
    return NULL;
}

void free_vhost_config(vhost_config *config) {
    if (!config) return;
    free(config->hostname);
    free(config->root);
    free(config->log_dir);
    free(config);
}
