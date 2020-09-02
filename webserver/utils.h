# ifndef __UTILS_H__
# define __UTILS_H__

#include <linux/limits.h>

char *fgets_or_exit(char *buffer, int size, FILE *stream);
char *rewrite_target(char *target);
char *check_root(char *root);
char * get_mime_type(char *name);
char *get_app_path(char *argv0);

#endif