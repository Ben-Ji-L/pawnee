#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include "http_parse.h"
#include "log.h"

// le fichier de log des requêtes
FILE *log_requests;

// le fichier de log des erreurs
FILE *log_errors;

/**
 * Initialisation du fichier de log des requêtes
 * @param path le chemin vers le dossier des logs
 */
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

/**
 * Initialisation du fichier de log des erreurs
 * @param path le chemin vers le dossier des logs
 */
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

/**
 * Fonction qui écrit une requête dans les logs
 * @param log_file le fichier dans lequel on doit écrire
 * @param request la requête à écrire
 * @param code le code HTTP de la requête
 */
void write_request(FILE *log_file, http_request request, int code) {
    char *method = "";
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);

    if (request.method == 0) {
        method = "GET";
    } else if (request.method == 1)
        method = "UNSUPPORTED";
    
    // le temps actuel
    struct tm *local = localtime(&now);

	hours = local->tm_hour;	  	// l'heure depuis minuit, de 0 à 23
	minutes = local->tm_min;	 	// les minutes, de 0 à 59
	seconds = local->tm_sec;	 	// les secondes, de 0 à 59

	day = local->tm_mday;			// le jour, de 1 à 31
	month = local->tm_mon + 1;   	// le mois, de 0 à 11
	year = local->tm_year + 1900;	// l'année depuis 1900

    // le formattage de la ligne de log
    fprintf(log_file, "[%02d/%02d/%d] [%02d:%02d:%02d] HTTP:%d/%d %d %s %s\n", day, month, year, hours, minutes, seconds, request.http_major, request.http_minor, code, method, request.target);
}

/**
 * Fonction qui écrit une erreur dans les logs
 * @param log_file le fichier dans lequel on doit écrire
 * @param error l'erreur à écrire
 */
void write_error(FILE *log_file, char *error) {
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);

    // le temps actuel
    struct tm *local = localtime(&now);

	hours = local->tm_hour;	  	// l'heure depuis minuit, de 0 à 23
	minutes = local->tm_min;	 	// les minutes, de 0 à 59
	seconds = local->tm_sec;	 	// les secondes, de 0 à 59

	day = local->tm_mday;			// le jour, de 1 à 31
	month = local->tm_mon + 1;   	// le mois, de 0 à 11
	year = local->tm_year + 1900;	// l'année depuis 1900

    // le formattage de la ligne de log
    fprintf(log_file, "[%02d/%02d/%d] [%02d:%02d:%02d] %s\n", day, month, year, hours, minutes, seconds, error);
}

/**
 * Renvoie un descripteur de fichier vers le fichier de log des requêtes
 */
FILE *get_log_requests(void) {
    return log_requests;
}

/**
 * Renvoie un descripteur de fichier vers le fichier de log des erreurs
 */
FILE *get_log_errors(void) {
    return log_errors;
}