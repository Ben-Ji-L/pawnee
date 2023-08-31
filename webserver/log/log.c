#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include "../http/http.h"
#include "../http/http_parse.h"
#include "log.h"
#include "../config/config.h"
#include "../file.h"

/** the log file for requests */
FILE *log_requests;

/** the log file for errors */
FILE *log_errors;

char client_ip[20];

/**
 * init the requests log file
 */
void create_requests_logs_file(void) {
    FILE *request_file;

    /* path to the request log file */
    char request_path[PATH_MAX];

    strncpy(request_path, get_config()->log_dir, PATH_MAX);
    strcat(request_path, "requests.log");

    request_file = fopen(request_path, "a");
    if (request_file == NULL) {
        perror("fopen request log error");
        exit(EXIT_FAILURE);
    }

    log_requests = request_file;
}

/**
 * init the errors log file
 */
void create_errors_logs_file(void) {
    FILE *errors_file;

    /* path to the errors log file */
    char error_path[PATH_MAX];

    strncpy(error_path, get_config()->log_dir, PATH_MAX);
    strcat(error_path, "errors.log");

    errors_file = fopen(error_path, "a");
    if (errors_file == NULL) {
        perror("fopen errors log error ");
        exit(EXIT_FAILURE);
    }

    log_errors = errors_file;
}

/**
 * function to write a request in the log file
 * @param log_file the log file
 * @param request the request to write
 * @param code the HTTP code of the request
 */
void write_request(FILE *log_file, http_request request, int code) {
    struct tm *local = get_actual_time();

    /* format the line to put in the log */
    fprintf(log_file, "[%02d/%02d/%d][%02d:%02d:%02d] IP:%s HTTP:%d/%d %d %s %s %s", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour,
            local->tm_min, local->tm_sec, \
            client_ip, request.http_major, request.http_minor, code, get_method(request.method),
            rewrite_target(request.target), request.headers[1]);
}

/**
 * function to write an error in the log file
 * @param log_file the log file
 * @param request the error to write
 */
void write_error(FILE *log_file, char *error) {
    struct tm *local = get_actual_time();

    /* format the line to put in the log */
    fprintf(log_file, "[%02d/%02d/%d][%02d:%02d:%02d] %s\n", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour,
            local->tm_min, local->tm_sec, error);
}

/**
 * return a file descriptor to the requests log file
 * @return the file descriptor
 */
FILE *get_log_requests(void) {
    return log_requests;
}

/**
 * return a file descriptor to the error log file
 * @return the file descriptor
 */
FILE *get_log_errors(void) {
    return log_errors;
}

struct tm *get_actual_time(void) {
    time_t now;
    time(&now);

    /* actual time */
    struct tm *local = localtime(&now);

    return local;
}