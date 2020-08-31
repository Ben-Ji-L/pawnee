#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <linux/limits.h>

#include "utils.h"
#include "http_parse.h"
#include "config.h"

/**
 * On lit des données et en cas d'erreur on quite le programme avec un statut d'erreur.
 * @param buffer Le buffer où l'on va stocker les données.
 * @param size La taille des données lues.
 * @param stream Le flux à partir duquel on va lire les données.
 * @return Le buffer.
 */
char *fgets_or_exit(char *buffer, int size, FILE *stream) {
	if (fgets(buffer, size, stream) == NULL) {
		perror("fgets error");
		exit(1);
	}
	return buffer;
}

/**
 * On ignore les en-tete de la requete.
 * @param client  Le stream de la requete.
 */
void skip_headers(FILE *client) {

	char data[512];
	do {
		fgets_or_exit(data, 512, client);
    } while (strncmp(data, "\r\n", 2) != 0);
}

/**
 * Fonction qui envoie au client le statut de la réponse
 * @param client Le flux où l'in va envoyer les données.
 * @param code Le code HTTP de la réponse.
 * @param reason_phrase La phrase qui accompagne le code HTTP.
 */
void send_status(FILE *client, int code, const char *reason_phrase) {
	fprintf(client, "HTTP/1.1 %d %s\r\n", code, reason_phrase);
}

/**
 * Dans cette fonction on met en forme la réponse HTTP.
 * @param client Le flux où écrire les données
 * @param code Le code HTTP de la réponse.
 * @param reason_phrase La phrase qui accompagne le code de la réponse.
 * @param message_body Le corps de la réponse.
 */
void send_response(FILE *client, int code, const char *reason_phrase, char *message_body, int size) {

    // On envoie la réponse en respectant la forme d'une réponse HTTP.
    send_status(client, code, reason_phrase);

	if (code == 200) {
		fprintf(client, "Content-Length: %d\r\n", size);
		fprintf(client, "Content-Type: %s\r\n", get_mime_type(message_body));
		fprintf(client, "\r\n");
		
	} else {
		fprintf(client, "Content-Length: %d\r\n", size);
		fprintf(client, "\r\n");
		fprintf(client, "%s\r\n", message_body);
	}
	
}

char *rewrite_target(char *target) {
	char *rewrited_target = strtok(strdup(target), "?");

	if (strcmp(rewrited_target, "/") == 0) {
		return "index.html";
	}

	return ++rewrited_target;
}

char *check_root(char *root) {
	if (access(root, R_OK | W_OK) != 0) {
		perror("no access to the root");
		exit(1);
	}

	struct stat root_stat;
	if (stat(root, &root_stat) != 0) {
		perror("root access error");
		exit(1);
	}

	if (!S_ISDIR(root_stat.st_mode)) {
		perror("root is not a directory");
		exit(1);
	} 

	if (strcmp(root+strlen(root)-1, "/") == 0)
		return root;
	
	return strcat(root, "/");
}

FILE *check_and_open(const char *target, const char *document_root) {
	char *path = strcat(strdup(document_root), target);
	struct stat path_stat;

	// Si stat echoue
	if (stat(path, &path_stat) != 0) {
		perror("stat error 1 ");
		return NULL;
	}

	if (!S_ISREG(path_stat.st_mode)) {
		perror("stat error 2 ");
		return NULL;
	}
	if (S_ISDIR(path_stat.st_mode)) {
		perror("stat error 3 ");
		return NULL;
	}

	FILE *result = fopen(path, "r");
	if (result == NULL) {
		perror("fopen error ");
		return NULL;
	}
	
	return result;
}

int get_file_size(int fd) {
	struct stat fd_stat;
	if (fstat(fd, &fd_stat) == 0) {
		return fd_stat.st_size;
	}
	return 0;
}

int copy(FILE *in, FILE *out) {
	char buff[1024];
	size_t s;

	while ((s = fread(buff, 1, 1024, in)) != 0)
		fwrite(buff, 1, s, out);

	return 0;
}

char *get_mime_type(char *name) {
	char *ext = strrchr(name, '.');
	char *delimiter = "*";
	char *mime_type = NULL;
	char *line;
	char *token = "";
	size_t len = 0;
    ssize_t read;
	ext++;

	FILE *mime_type_file = fopen(get_config()->mimes_file, "r");
	if (mime_type_file != NULL) {
		while ((read = getline(&line, &len, mime_type_file)) != -1) {
			line[strlen(line)-1] = '\0';
        	strtok(line, delimiter);
			if ((token = strtok(NULL, delimiter)) != NULL) {
				if (strcmp(token, ext) == 13) {
					return strtok(line, delimiter);
				}
			}
		}
		fclose(mime_type_file);
	} else {
		perror("open mime file error : ");
	}

	return mime_type;
}

char *get_app_path(char *argv0) {
	char abs_exe_path[PATH_MAX];
	char path_save[PATH_MAX];
    char *p;
	char *result;

    if(!(p = strrchr(argv0, '/')))
        getcwd(abs_exe_path, sizeof(abs_exe_path));
    else {
        *p = '\0';
        getcwd(path_save, sizeof(path_save));
        chdir(argv0);
        getcwd(abs_exe_path, sizeof(abs_exe_path));
        chdir(path_save);
    }
	result = &abs_exe_path[0];
	return result;
}