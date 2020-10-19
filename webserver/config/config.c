#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "config.h"
#include "../log/log.h"

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
int init_config(char *abs_path) {
    char types[PATH_MAX];
    strcpy(types, abs_path);

    // init the shared memory zone
    shared_mem_config = mmap(NULL, sizeof(config), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem_config == MAP_FAILED) {
        write_error(get_log_errors(), "mmap config error");
        return 1;
    }

    // if we don't read the file the structure is set with default values
    config.port = 8080;
    strcpy(config.listen_addr, "127.0.0.1");
    strcat(types, "/types.txt");
    strcpy(config.mimes_file, types);
    get_config_from_file(abs_path);

    // copy config structure in the shared memory
    memmove(shared_mem_config, &config, sizeof(server_config));

    return 0;
}

/**
 * open the config file and read it
 * @param abs_path absolute path of the application
 * @return 0 on success, 1 on error
 */
int get_config_from_file(char *abs_path) {
    FILE *config_file;

    char path[PATH_MAX];
    strcpy(path, abs_path);

    int bufferLength = 255;
    char buffer[bufferLength];

    // try to open config file
    if ((config_file = fopen(strcat(path, "/../config/server.cfg"), "r")) == NULL) {
        write_error(get_log_errors(), "open config file error");
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