#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "http_parse.h"

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
void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body) {

    // On envoie la réponse en respectant la forme d'une réponse HTTP.
    send_status(client, code, reason_phrase);
	fprintf(client, "Content-size: %d\r\n", (int) strlen(message_body));
	fprintf(client, "\r\n");
	fprintf(client, "%s", message_body);
}