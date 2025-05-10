#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

#include "http.h"
#include "http_headers.h"
#include "../file.h"
#include "../log/log.h"
#include "../vhosts/hosts.h"

int check_http_version(http_request *request) {
    // we not support HTTP 2 yet
    if (request->http_major > 1) {
        return 1;
    }
    return 0;
}

/**
 * send the status of the response
 * @param client stream to send data
 * @param code HTTP code of the response
 * @param reason_phrase HTTP response reason phrase
 */
void send_status(int fd, http_request *request, int code, const char *reason_phrase) {
    char status_line[128];
    int len = snprintf(status_line, sizeof(status_line), "HTTP/%d.%d %d %s\r\n",
                       request->http_major, request->http_minor, code, reason_phrase);
    write(fd, status_line, len);
}

/**
 * function to format the HTTP response
 * @param client stream to write the response
 * @param code the HTTP code of the response
 * @param reason_phrase the reason phrase of the response
 * @param message_body the body of the response
 * @param size the size of the response body
 */
void send_response(FILE *client, http_request *request, int code, const char *reason_phrase, char *message_body, int size) {
    int fd = fileno(client);

    // Envoi de la ligne de statut HTTP
    send_status(fd, request, code, reason_phrase);

    // Date HTTP
    char *date = get_date_http_format();

    // En-têtes communs à toutes les réponses
    char header[2048];  // Taille du tampon augmentée pour éviter tout débordement
    int header_len = snprintf(header, sizeof(header),
        "Server: Pawnee\r\n"
        "Date: %s\r\n", date);

    // Ajouter d'autres en-têtes en fonction du code de réponse
    if (code == 200) {
        const char *mime = get_mime_type(message_body);
        if (!mime) mime = "application/octet-stream";  // Valeur par défaut

        header_len += snprintf(header + header_len, sizeof(header) - header_len,
            "Content-Length: %d\r\n"
            "Accept-Ranges: bytes\r\n", size);

        if (strncmp(mime, "text/", 5) == 0) {
            header_len += snprintf(header + header_len, sizeof(header) - header_len,
                "Content-Type: %s; charset=utf-8\r\n", mime);
        } else {
            header_len += snprintf(header + header_len, sizeof(header) - header_len,
                "Content-Type: %s\r\n", mime);
        }
    } else if (code == 405) {
        header_len += snprintf(header + header_len, sizeof(header) - header_len,
            "Allow: GET, HEAD\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n", size);
    } else {
        header_len += snprintf(header + header_len, sizeof(header) - header_len,
            "Content-Length: %d\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n", size);
    }

    // Ajouter la ligne vide pour séparer les en-têtes du corps
    snprintf(header + header_len, sizeof(header) - header_len, "\r\n");

    // Envoyer les en-têtes HTTP
    ssize_t sent_header = write(fileno(client), header, strlen(header));
    if (sent_header < 0) {
        perror("Failed to send HTTP headers");
    }
    
    // Ne pas envoyer le corps pour une méthode HEAD
    if (request->method != HTTP_HEAD) {
        if (code == 200) {
            // Envoyer le fichier si c'est un code 200
            char *vhost_root = get_vhost_root(request);
            FILE *file = check_and_open(message_body, vhost_root);
            free(vhost_root);

            if (file) {
                char buffer[8192];
                size_t read_bytes;
                while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    ssize_t sent_file = write(fileno(client), buffer, read_bytes);
                    if (sent_file < 0) {
                        perror("Failed to send file content");
                    }
                }
                fclose(file);
            } else {
                perror("Failed to open file");
            }
        } else {
            // Pour les autres codes (405, etc.), envoyer simplement le message du corps
            ssize_t sent_body = write(fileno(client), message_body, size);
            if (sent_body < 0) {
                perror("Failed to send body");
            }
        }
    }

    // Assurer que tout a été envoyé
    fflush(client);
    free_headers(&request->headers);
    free(date);
}

/**
 * return actual date of the server with the correct format for HTTP response
 * @return well formatted date
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
 * rewrite the HTTP target within URLs variables
 * @param target the target of the request
 * @return well rewrite target
 */
char *rewrite_target(char *target) {
    char *rewrite_target;
    if ((rewrite_target = strtok(strdup(target), "?")) == NULL) {
        write_error(get_log_errors(), "rewrite target error");
        exit(EXIT_FAILURE);
    }

    if (strcmp(rewrite_target, "/") == 0) {
        return "index.html";
    }

    return ++rewrite_target;
}

char *get_query_params(char *target) {
    char *query_params;

    if ((query_params = strtok(strdup(target), "?")) == NULL) {
        return NULL;
    }
    query_params = strtok(NULL, "?");
    return query_params;
}
