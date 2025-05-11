#ifndef __HOSTS_H__
#define __HOSTS_H__

#include "../http/http_parse.h"
#include "../config/config.h"

vhost_config *get_vhost_config(http_request *request);

void free_vhost_config(vhost_config *config);

#endif
