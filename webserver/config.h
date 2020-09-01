#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <linux/limits.h>

// Structure représentant la configuration du serveur
typedef struct {
    int port;
    char listen_addr[PATH_MAX];
    char website_root[PATH_MAX];
    char mimes_file[PATH_MAX];
} server_config;

int init_config(char *abs_path);
int get_config_from_file(char *abs_path);
server_config *get_config(void);
#endif