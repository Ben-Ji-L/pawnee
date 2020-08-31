#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>

#include "config.h"
#include "utils.h"

static server_config config;
server_config *shared_mem_config;

int init_config(char *abs_path) {
    char types[PATH_MAX];
    strcpy(types, abs_path);

    shared_mem_config = mmap(NULL, sizeof(config), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem_config == MAP_FAILED) {
        perror("mmap config error");
        exit(EXIT_FAILURE);
    }

    config.port = 8080;
    config.listen_addr = "127.0.0.1";
    config.website_root = "web/";

    strcat(types, "/types.txt");
    config.mimes_file = types;
    memcpy(shared_mem_config, &config, sizeof(config));

    get_config_from_file(abs_path);

    return 0;
}

int get_config_from_file(char *abs_path) {
    FILE *config_file;

    char path[256];
    strcpy(path, abs_path);

    if ((config_file = fopen(strcat(path, "/server.cfg"), "r")) == NULL) {
        perror("open config file error");
        exit(EXIT_FAILURE);
    }

    fclose(config_file);

    return 0;
}

server_config *get_config(void) {
    return shared_mem_config;
}