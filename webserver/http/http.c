#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

#include "http.h"
#include "../file.h"

/**
 * On ignore les en-tête de la requête.
 * @param client  Le stream de la requête.
 */
void skip_headers(FILE *client, http_request *request) {

    char data[512];
    int i = 0;

    do {
        request->headers[i] = fgets_or_exit(data, 512, client);
        i++;
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

    switch (code) {
        case 200:
            fprintf(client, "Content-Length: %d\r\n", size);

            if (strncmp(get_mime_type(message_body), "text/", strlen("text/")) == 0) {
                fprintf(client, "Content-Type: %s; charset=utf-8\r\n", get_mime_type(message_body));
            } else
                fprintf(client, "Content-Type: %s\r\n", get_mime_type(message_body));

            fprintf(client, "Accept-Ranges: bytes\r\n");
            fprintf(client, "Date: %s\r\n", get_date_http_format());
            fprintf(client, "\r\n");
            break;

        case 405:
            fprintf(client, "Allow: GET, HEAD\r\n");
            fprintf(client, "\r\n");
            break;

        default:
            fprintf(client, "Content-Length: %d\r\n", size);
            fprintf(client, "\r\n");
            fprintf(client, "%s\r\n", message_body);
            break;
    }
}

/**
 * Renvoie la date actuelle du serveur formattée pour les en-tetes Date du protocole HTTP
 * @return la date correctement formatée
 */
char *get_date_http_format(void) {
    time_t rawtime;
    struct tm *timeinfo;
    char *date = malloc(100);

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(date, 40, "%a, %d %b %G %X %Z", timeinfo);
    return date;
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