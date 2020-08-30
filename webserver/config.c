#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>

#include "config.h"
#include "utils.h"

static server_config config;
server_config *shared_mem_config;

int init_config(void) {

    shared_mem_config = mmap(NULL, sizeof(config), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem_config == MAP_FAILED) {
        perror("mmap config error");
        exit(EXIT_FAILURE);
    }

    config.port = 8080;
    config.listen_addr = "127.0.0.1";
    config.website_root = "web/";
    config.mimes_file = strcat(abs_exe_path, "/types.txt");

    memcpy(shared_mem_config, &config, sizeof(config));

    return 0;
}

int get_config_from_file(void) {
    return 0;
}

server_config *get_config(void) {
    return shared_mem_config;
}