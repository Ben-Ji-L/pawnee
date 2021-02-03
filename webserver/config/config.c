#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "config.h"
#include "../log/log.h"
#include "../file.h"

// configuration of the server
static server_config config;

/**
 * shared memory zone for access config in the whole application
 */
server_config *shared_mem_config;

/**
 * Init the configuration of the server
 * @param abs_path absolute path of the app
 * @return 0 on success, 1 on error
 */
int init_config() {
    char types[PATH_MAX] = "";

    // init the shared memory zone
    shared_mem_config = mmap(NULL, sizeof(config), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem_config == MAP_FAILED) {
        perror("config shared memory error");
        return 1;
    }

    // if we don't read the file the structure is set with default values
    config.port = 8080;
    strcpy(config.listen_addr, "127.0.0.1");
    strcat(types, "/types.txt");
    strcpy(config.mimes_file, types);

    char *log_path = get_app_path();
    strncat(log_path, "/logs/", PATH_MAX);

    strcpy(config.log_dir, "/logs/");

    if (get_config_from_file() != 0) {
        perror("file config loading error");
        return 1;
    }

    // copy config structure in the shared memory
    memmove(shared_mem_config, &config, sizeof(server_config));

    return 0;
}

/**
 * open the config file and read it
 * @param abs_path absolute path of the application
 * @return 0 on success, 1 on error
 */
int get_config_from_file(void) {
    FILE *config_file;

    char path[PATH_MAX];
    strcpy(path, get_app_path());
    strcat(path, "../config/server.cfg");

    int bufferLength = 255;
    char buffer[bufferLength];

    // try to open config file
    if ((config_file = fopen(path, "r")) == NULL) {
        perror("open config file error");
        return 1;
    }

    /*
     * read a line
     * if it starts with '#' we jump to the next one
     * if it contains '=' we read the key and it's value
     */
    while (fgets(buffer, bufferLength, config_file)) {

        // processing of the line
        if (buffer[0] != '#') {
            char *token = strtok(buffer, "=");

            // if the line is the port to listen
            if (strcmp(token, "port") == 0) {
                token = strtok(NULL, "=");
                config.port = atoi(token);

                // if the line is the ip address to listen
            } else if (strcmp(token, "listen_addr") == 0) {
                token = strtok(NULL, "=");
                token = strtok(token, "\"");
                strcpy(config.listen_addr, token);

                // if the line is the web root
            } else if (strcmp(token, "mimes_file") == 0) {
                token = strtok(NULL, "=");
                token = strtok(token, "\"");
                strcpy(config.mimes_file, token);
            } else if (strcmp(token, "logs_dir") == 0) {
                token = strtok(NULL, "=");
                token = strtok(token, "\"");
                strcpy(config.log_dir, token);
            }
        }
    }
    fclose(config_file);
    return 0;
}

/**
 * return server config
 * @return a pointer to the shared memory zone with the configuration of the server inside
 */
server_config *get_config(void) {
    return shared_mem_config;
}