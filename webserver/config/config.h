#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <limits.h>

/**
 * Structure représentant la configuration du serveur
 */
typedef struct {

    /**
     * Le port sur lequel écoute le serveur.
     */
    int port;

    /**
     * L'adresse ip sur laquelle le serveur écoute.
     */
    char listen_addr[PATH_MAX];

    /**
     * La racine du site que le serveur sert.
     */
    char website_root[PATH_MAX];

    /**
     * Le fichier utilisé pour déterminer les types mimes suportés.
     */
    char mimes_file[PATH_MAX];
} server_config;

int init_config(char *abs_path);

int get_config_from_file(char *abs_path);

server_config *get_config(void);

#endif