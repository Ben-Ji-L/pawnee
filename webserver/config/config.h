#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <limits.h>

/** struct for saving the configuration of the server */
typedef struct {

    /** port to listen */
    int port;

    /** ip address to listen */
    char listen_addr[PATH_MAX];

    /** web root of the site to serve */
    char website_root[PATH_MAX];

    /** mime types file path */
    char mimes_file[PATH_MAX];
} server_config;

int init_config(char *abs_path);

int get_config_from_file(char *abs_path);

server_config *get_config(void);

#endif