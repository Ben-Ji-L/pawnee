#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <libgen.h>

#include "config/config.h"
#include "log/log.h"

/**
 * function to check if the file of the target exist,
 * check if we can open it and open it
 * @param target the target of the request
 * @param document_root the root path of the website
 * @return a pointer to the opened file
 */
FILE *check_and_open(const char *target, const char *document_root) {
    char root[PATH_MAX] = "";
    char path[PATH_MAX] = "";

    struct stat path_stat;

    // prepare the file before opening it
    strncpy(root, document_root, PATH_MAX);
    strncat(path, root, PATH_MAX);
    strncat(path, target, PATH_MAX);

    // if stat fail
    if (stat(path, &path_stat) != 0) {
        write_error(get_log_errors(), "stat error: no such file or directory");
        return NULL;
    }

    if (!S_ISREG(path_stat.st_mode)) {
        write_error(get_log_errors(), "stat error: not regular file");
        return NULL;
    }
    if (S_ISDIR(path_stat.st_mode)) {
        write_error(get_log_errors(), "stat error: is a directory");
        return NULL;
    }

    FILE *result = fopen(path, "r");
    if (result == NULL) {
        write_error(get_log_errors(), "fopen error ");
        return NULL;
    }

    return result;
}

/**
 * find the size of a file
 * @param fd the file descriptor for the file to open
 * @return the size of the file
 */
int get_file_size(int fd) {
    struct stat fd_stat;
    if (fstat(fd, &fd_stat) == 0) {
        return fd_stat.st_size;
    }
    return 0;
}

/**
 * copy the content of the file to another
 * @param in the file to read
 * @param out the file to copy data
 */
void copy(FILE *in, FILE *out) {
    char buff[1024];
    size_t s;

    while ((s = fread(buff, 1, 1024, in)) != 0)
        fwrite(buff, 1, s, out);
}

/**
 * read data and if it fail quit the program with error code
 * @param buffer buffer to store data
 * @param size the size of data we read
 * @param stream the stream to read data
 * @return the buffer
 */
char *fgets_or_exit(char *buffer, int size, FILE *stream) {
    if (fgets(buffer, size, stream) == NULL) {
        write_error(get_log_errors(), "fgets error");
        return NULL;
    }
    return buffer;
}

/**
 * check if we can open the root of the website
 * @param root the path to the root
 * @return the root after the check
 */
char *check_root(char *root) {
    if (access(root, R_OK | W_OK) != 0) {
        write_error(get_log_errors(), "no access to the root");
        exit(1);
    }

    struct stat root_stat;
    if (stat(root, &root_stat) != 0) {
        write_error(get_log_errors(), "root access error");
        exit(1);
    }

    if (!S_ISDIR(root_stat.st_mode)) {
        write_error(get_log_errors(), "root is not a directory");
        exit(1);
    }

    if (strcmp(root + strlen(root) - 1, "/") == 0)
        return root;

    return strcat(root, "/");
}

/**
 * return the mime type of a file
 * @param name the name of the file
 * @return the mime type of the file
 */
char *get_mime_type(char *name) {
    char *ext = strrchr(name, '.');
    char *delimiter = "*";
    char *mime_type = NULL;
    char *token = "";
    size_t len = 0;
    char *line = malloc(sizeof(len));
    ssize_t read = 0;
    ext++;

    // open mime type file
    FILE *mime_type_file = fopen(get_config()->mimes_file, "r");

    if (mime_type_file != NULL) {
        while ((read = getline(&line, &len, mime_type_file)) != -1) {
            line[strlen(line) - 1] = '\0';
            strtok(line, delimiter);
            if ((token = strtok(NULL, delimiter)) != NULL) {

                // remove the final space
                token[strlen(token) - 1] = '\0';

                if (strcmp(token, ext) == 0) {
                    char *result = strtok(line, delimiter);

                    // if a line has a end space remove it
                    if (strcmp(&result[strlen(result) - 1], " ") == 0)
                        result[strlen(result - 1)] = '\0';

                    return result;
                }
            }
        }
        fclose(mime_type_file);

    } else {
        write_error(get_log_errors(), "open mime file error : ");
    }

    return mime_type;
}

/**
 * return the path of the application
 * @param argv0 the path of the executable
 * @return absolute path of the file
 */
char *get_app_path(void) {
    char *path = malloc(PATH_MAX);
    if (readlink("/proc/self/exe", path, PATH_MAX) != -1) {
        dirname(path);
        strcat(path, "/");
    }

    return path;
}