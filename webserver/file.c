#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <linux/limits.h>

#include "log.h"

/**
 * Fonction qui vérifie si un fichier existe, si on peut l'ouvrir
 * et ouvre un fichier
 * @param target la cible de la requête
 * @param document_root le répertoire racine du site à servir
 * @return un pointeur vers le fichier ouvert
 */
FILE *check_and_open(const char *target, const char *document_root) {
	char *path = strcat(strdup(document_root), target);
	struct stat path_stat;

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