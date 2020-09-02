#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <time.h>

#include "http_parse.h"
#include "log.h"

FILE *log_requests;
FILE *log_errors;

void create_requests_logs_file(char *path) {
    FILE *request_log;
    char request_path[PATH_MAX];

    // chemin des logs globaux
    strcat(path, "/logs/");

    // chemin du log des requêtes
    strcpy(request_path, path);
    strcat(request_path, "requests.log");

    request_log = fopen(request_path, "a");
    if (request_log == NULL) {
        write_error(get_log_errors(), "fopen request log error");
		exit(EXIT_FAILURE);
    }

    log_requests = request_log;
}

void create_errors_logs_file(char *path) {
    FILE *error_log;
    char error_path[PATH_MAX];
    
    // chemin du log des erreurs
    strcpy(error_path, path);
    strcat(error_path, "errors.log");

    error_log = fopen(error_path, "a");
    if (error_log == NULL) {
        write_error(get_log_errors(), "fopen errors log error ");
		exit(EXIT_FAILURE);
    }

    log_errors = error_log;
}

void write_request(FILE *log_file, http_request request, int code) {
    char *method = "";
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);

    if (request.method == 0) {
        method = "GET";
    } else if (request.method == 1)
        method = "UNSUPPORTED";
    
    struct tm *local = localtime(&now);

	hours = local->tm_hour;	  	// get hours since midnight (0-23)
	minutes = local->tm_min;	 	// get minutes passed after the hour (0-59)
	seconds = local->tm_sec;	 	// get seconds passed after minute (0-59)

	day = local->tm_mday;			// get day of month (1 to 31)
	month = local->tm_mon + 1;   	// get month of year (0 to 11)
	year = local->tm_year + 1900;	// get year since 1900

    fprintf(log_file, "[%02d/%02d/%d] [%02d:%02d:%02d] HTTP:%d/%d %d %s %s\n", day, month, year, hours, minutes, seconds, request.http_major, request.http_minor, code, method, request.target);
}

void write_error(FILE *log_file, char *error) {
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);

    struct tm *local = localtime(&now);

	hours = local->tm_hour;	  	// get hours since midnight (0-23)
	minutes = local->tm_min;	 	// get minutes passed after the hour (0-59)
	seconds = local->tm_sec;	 	// get seconds passed after minute (0-59)

	day = local->tm_mday;			// get day of month (1 to 31)
	month = local->tm_mon + 1;   	// get month of year (0 to 11)
	year = local->tm_year + 1900;	// get year since 1900

    fprintf(log_file, "[%02d/%02d/%d] [%02d:%02d:%02d] %s\n", day, month, year, hours, minutes, seconds, error);
}

FILE *get_log_requests(void) {
    return log_requests;
}

FILE *get_log_errors(void) {
    return log_errors;
}