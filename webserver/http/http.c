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
#include "../log/log.h"

/**
 * ignore headers of yhe requests
 * @param client request stream
 * @param request request we parse
 */
void skip_and_save_headers(FILE *client, http_request *request) {

    char data[512];
    int i = 0;
    do {
        request->headers[i] = malloc(512);
        strncpy(request->headers[i], fgets_or_exit(data, 512, client), 512);
        if (request->headers[i] == NULL) {
            write_error(get_log_errors(), "read header error");
            exit(EXIT_FAILURE);
        }
        i++;
    } while (strncmp(data, "\r\n", 2) != 0);
}

/**
 * check host header
 * @param request the request to check headers
 * @return 0 if everything ok, 1 otherwise
 */
int check_host_header(http_request *request) {
    char header[512];

    strcpy(header, request->headers[0]);
    if (strcmp(strtok(header, ":"), "Host") == 0)
        return 0;
    return 1;
}

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
void send_status(FILE *client, http_request *request, int code, const char *reason_phrase) {
    fprintf(client, "HTTP/%d.%d %d %s\r\n", request->http_major, request->http_minor, code, reason_phrase);
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

    /* we send the status of the response */
    send_status(client, request, code, reason_phrase);

    char *date;
    date = get_date_http_format();

    switch (code) {
        case 200:
            fprintf(client, "Content-Length: %d\r\n", size);

            if (strncmp(get_mime_type(message_body), "text/", strlen("text/")) == 0) {
                fprintf(client, "Content-Type: %s; charset=utf-8\r\n", get_mime_type(message_body));
            } else
                fprintf(client, "Content-Type: %s\r\n", get_mime_type(message_body));

            fprintf(client, "Accept-Ranges: bytes\r\n");
            fprintf(client, "Date: %s\r\n", date);
            fprintf(client, "\r\n");
            break;

        case 405:
            fprintf(client, "Allow: GET, HEAD\r\n");
            fprintf(client, "Date: %s\r\n", date);
            fprintf(client, "\r\n");
            break;

        default:
            fprintf(client, "Content-Length: %d\r\n", size);
            fprintf(client, "Date: %s\r\n", date);
            fprintf(client, "\r\n");
            fprintf(client, "%s\r\n", message_body);
            break;
    }
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
