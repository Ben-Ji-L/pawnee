#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <limits.h>

/** struct for saving the configuration of the server */
typedef struct {

    /** port to listen */
    int port;

    /** ip address to listen */
    char listen_addr[PATH_MAX];

    /** mime types file path */
    char mimes_file[PATH_MAX];

    char log_dir[PATH_MAX];
} server_config;

int init_config();

int get_config_from_file();

server_config *get_config(void);

#endif