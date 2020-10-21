#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include "../http/http.h"
#include "../http/http_parse.h"
#include "log.h"

/** the log file for requests */
FILE *log_requests;

/** the log file for errors */
FILE *log_errors;

/**
 * init the requests log file
 * @param path the path to the log directory
 */
void create_requests_logs_file(char *path) {
    FILE *request_log;
    char request_path[PATH_MAX];

    /* path to global log path */
    strcat(path, "../logs/");

    /* path to requests log file */
    strcpy(request_path, path);
    printf("path: %s\n", request_path);
    strcat(request_path, "requests.log");

    request_log = fopen(request_path, "a");
    if (request_log == NULL) {
        perror("fopen request log error");
        exit(EXIT_FAILURE);
    }

    log_requests = request_log;
}

/**
 * init the errors log file
 * @param path the path to the log directory
 */
void create_errors_logs_file(char *path) {
    FILE *error_log;
    char error_path[PATH_MAX];

    /* path to the errors log file */
    strcpy(error_path, path);
    strcat(error_path, "errors.log");

    error_log = fopen(error_path, "a");
    if (error_log == NULL) {
        perror("fopen errors log error ");
        exit(EXIT_FAILURE);
    }

    log_errors = error_log;
}

/**
 * function to write a request in the log file
 * @param log_file the log file
 * @param request the request to write
 * @param code the HTTP code of the request
 */
void write_request(FILE *log_file, http_request request, int code) {
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);

    /* actual time */
    struct tm *local = localtime(&now);

    hours = local->tm_hour;        // time since midnight, from 0 to 23
    minutes = local->tm_min;        // minutes, from 0 to 59
    seconds = local->tm_sec;        // seconds, from 0 to 59

    day = local->tm_mday;            // the day, from 1 to 31
    month = local->tm_mon + 1;    // the month, from 0 to 11
    year = local->tm_year + 1900;    // the years since 1900

    /* format the line to put in the log */
    fprintf(log_file, "[%02d/%02d/%d][%02d:%02d:%02d] IP:%s HTTP:%d/%d %d %s %s\n", day, month, year, hours,
            minutes, seconds, \
            client_ip, request.http_major, request.http_minor, code, get_method(request.method), rewrite_target(request.target));
}

/**
 * function to write an error in the log file
 * @param log_file the log file
 * @param request the error to write
 */
void write_error(FILE *log_file, char *error) {
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);

    /* actual time */
    struct tm *local = localtime(&now);

    hours = local->tm_hour;        // time since midnight, from 0 to 23
    minutes = local->tm_min;        // minutes, from 0 to 59
    seconds = local->tm_sec;        // seconds, from 0 to 59

    day = local->tm_mday;            // the day, from 1 to 31
    month = local->tm_mon + 1;    // the month, from 0 to 11
    year = local->tm_year + 1900;    // the years since 1900

    /* format the line to put in the log */
    fprintf(log_file, "[%02d/%02d/%d][%02d:%02d:%02d] %s\n", day, month, year, hours, minutes, seconds, error);
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