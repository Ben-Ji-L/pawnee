#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <linux/limits.h>

#include "config.h"
#include "log.h"

/**
 * Fonction qui vérifie si un fichier existe, si on peut l'ouvrir
 * et ouvre un fichier
 * @param target la cible de la requête
 * @param document_root le répertoire racine du site à servir
 * @return un pointeur vers le fichier ouvert
 */
FILE *check_and_open(const char *target, const char *document_root) {
	char path[PATH_MAX];
	struct stat path_stat;

	strcpy(path, strcat(strdup(document_root), target));

	// Si stat échoue
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
 * Calcule la taille d'un fichier
 * @param fd le descripteur vers le fichier
 * @return la taille du fichier
 */
int get_file_size(int fd) {
	struct stat fd_stat;
	if (fstat(fd, &fd_stat) == 0) {
		return fd_stat.st_size;
	}
	return 0;
}

/**
 * Copie le contenu d'un fichier vers un autre
 * @param in le fichier d'où on lit les données
 * @param out le ficher vers lequel on copie les données
 */
void copy(FILE *in, FILE *out) {
	char buff[1024];
	size_t s;

	while ((s = fread(buff, 1, 1024, in)) != 0)
		fwrite(buff, 1, s, out);
}

/**
 * On lit des données et en cas d'erreur on quite le programme avec un statut d'erreur.
 * @param buffer Le buffer où l'on va stocker les données.
 * @param size La taille des données lues.
 * @param stream Le flux à partir duquel on va lire les données.
 * @return Le buffer.
 */
char *fgets_or_exit(char *buffer, int size, FILE *stream) {
	if (fgets(buffer, size, stream) == NULL) {
		write_error(get_log_errors(), "fgets error");
		exit(1);
	}
	return buffer;
}

/**
 * Vérifie si on peut ouvrir le dossier racine du site
 * @param root le chemin vers la racine du site web
 * @return la racine du site vérifiée et corrigée
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

	if (strcmp(root+strlen(root)-1, "/") == 0)
		return root;
	
	return strcat(root, "/");
}

/**
 * Renvoie le type mime d'un fichier
 * @param name le nom du fichier dont on veut le type
 * @return le type mime du fichier
 */
char *get_mime_type(char *name) {
	char *ext = strrchr(name, '.');
	char *delimiter = "*";
	char *mime_type = NULL;
	char *line;
	char *token = "";
	size_t len = 0;
    ssize_t read;
	ext++;

	// Ouvre le fichier des types mimes
	FILE *mime_type_file = fopen(get_config()->mimes_file, "r");

	if (mime_type_file != NULL) {
		while ((read = getline(&line, &len, mime_type_file)) != -1) {
			line[strlen(line)-1] = '\0';
        	strtok(line, delimiter);
			if ((token = strtok(NULL, delimiter)) != NULL) {

				// suppression de l'espace final
				token[strlen(token)-1] = '\0';

				if (strcmp(token, ext) == 0) {
					char *result = strtok(line, delimiter);

					// si il y a un espace en fin de ligne, le supprime
					if (strcmp(&result[strlen(result)-1], " ") == 0)
						result[strlen(result-1)] = '\0';

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
 * Renvoie le chemin de l'application
 * @param argv0 le chemin de l'executable
 * @return le chemin de absolu de l'application
 */
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

	result = abs_exe_path;
	return result;
}