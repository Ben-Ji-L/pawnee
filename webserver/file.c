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

    // Vérifie que le document_root et target sont valides
    if (document_root == NULL || strlen(document_root) == 0) {
        write_error(get_log_errors(), "Invalid document root");
        return NULL;
    }

    // Prépare le chemin complet
    strncpy(root, document_root, PATH_MAX - 1);  // Pour éviter l'overflow
    root[PATH_MAX - 1] = '\0';  // S'assurer que root est bien terminé

    // Assure-toi qu'il y a un séparateur "/" entre root et target
    if (root[strlen(root) - 1] != '/') {
        strncat(root, "/", PATH_MAX - strlen(root) - 1);
    }

    // Crée le chemin complet
    strncat(path, root, PATH_MAX - strlen(path) - 1);  // Ajouter le root au path
    strncat(path, target, PATH_MAX - strlen(path) - 1);  // Ajouter le target au path
    path[PATH_MAX - 1] = '\0';  // S'assurer que path est bien terminé

    // Vérification de l'existence du fichier
    if (stat(path, &path_stat) != 0) {
        write_error(get_log_errors(), "stat error: no such file or directory");
        return NULL;
    }

    // Vérification si c'est un fichier régulier
    if (!S_ISREG(path_stat.st_mode)) {
        write_error(get_log_errors(), "stat error: not regular file");
        return NULL;
    }

    // Vérification si c'est un répertoire
    if (S_ISDIR(path_stat.st_mode)) {
        write_error(get_log_errors(), "stat error: is a directory");
        return NULL;
    }

    // Ouverture du fichier
    FILE *result = fopen(path, "r");
    if (result == NULL) {
        write_error(get_log_errors(), "fopen error");
        return NULL;
    }

    // Retourne le fichier ouvert
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
void copy(FILE *src, FILE *dest) {
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dest);
        if (bytes_written != bytes_read) {
            perror("Error writing to client");
            break;
        }
        fflush(dest);  // Force l'écriture immédiatement
    }
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
        exit(EXIT_FAILURE);
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

    return strncat(root, "/", PATH_MAX);
}

/**
 * return the mime type of a file
 * @param name the name of the file
 * @return the mime type of the file
 */
char *get_mime_type(char *name) {
    char *ext = strrchr(name, '.');
    char *mime_type = "";
    size_t len = 0;
    char *line = malloc(sizeof(len));
    ext++;

    // open mime type file
    FILE *mime_type_file = fopen(get_config()->mimes_file, "r");

    if (mime_type_file != NULL) {
        char *delimiter = "*";
        ssize_t read = 0;

        while ((read = getline(&line, &len, mime_type_file)) != -1) {
            char *token = "";

            line[strlen(line) - 1] = '\0';
            strtok(line, delimiter);
            if ((token = strtok(NULL, delimiter)) != NULL) {

                // remove the final space
                token[strlen(token)] = '\0';

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
 * @return absolute path of the file with a \0 at the end
 */
char *get_app_path(void) {
    char *path = malloc(PATH_MAX);
    if (path == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (len == -1) {
        perror("readlink error");
        free(path);
        exit(EXIT_FAILURE);
    }
    path[len] = '\0';  // Null-terminate the path

    char *dir = dirname(path);
    if (dir == NULL) {
        perror("dirname error");
        free(path);
        exit(EXIT_FAILURE);
    }

    // Copy the directory path back to the original path buffer
    strncpy(path, dir, PATH_MAX - 1);
    path[PATH_MAX - 1] = '\0';  // Ensure null-termination

    return path;
}