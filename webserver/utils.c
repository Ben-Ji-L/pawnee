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
#include "log.h"

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
 * Fonction qui réecrit la requête en enlevant les variables (après le ?)
 * et qui transforme la requête "/" en "index.html"
 * @param target la requête à examiner
 * @return la requête réécrite
 */
char *rewrite_target(char *target) {
	char *rewrited_target = strtok(strdup(target), "?");

	if (strcmp(rewrited_target, "/") == 0) {
		return "index.html";
	}

	return ++rewrited_target;
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
				if (strcmp(token, ext) == 13) {
					return strtok(line, delimiter);
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

	result = &abs_exe_path[0];
	return result;
}