#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct {
    int port;
    char *listen_addr;
    char *website_root;
    char *mimes_file;
} server_config;

int init_config(char *abs_path);
int get_config_from_file(char *abs_path);
server_config *get_config(void);
#endif