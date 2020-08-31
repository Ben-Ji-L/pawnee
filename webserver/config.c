#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

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

    // si le fichie n'est pas lu on place les valeurs par défaut dans la structure
    if (get_config_from_file(abs_path) != 0) {
        config.port = 8080;
    }
    config.listen_addr = "127.0.0.1";
    config.website_root = "web/";

    strcat(types, "/types.txt");
    config.mimes_file = types;

    memcpy(shared_mem_config, &config, sizeof(config));

    return 0;
}

int get_config_from_file(char *abs_path) {
    FILE *config_file;

    char path[256];
    strcpy(path, abs_path);

    int bufferLength = 255;
    char buffer[bufferLength];

    if ((config_file = fopen(strcat(path, "/server.cfg"), "r")) == NULL) {
        perror("open config file error");
        return 1;
    }

    // on lit une ligne
    // si elle commence par un # on passe à la suivante
    // si elle contient un = on lit la clé et la valeur
    // si partie gauche = port on stocke la valeur de droite dans config.port
    while(fgets(buffer, bufferLength, config_file)) {

        // traitement de la ligne
        if (buffer[0] != '#') {
            char *token = strtok(buffer, "=");
            
            if (strcmp(token, "port") == 0) {
                token = strtok(NULL, "=");

                config.port = atoi(token);
            }
        }
    }
    fclose(config_file);

    return 0;
}

server_config *get_config(void) {
    return shared_mem_config;
}