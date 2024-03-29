# ifndef __LOG_H__
#define __LOG_H__

#include "../http/http_parse.h"

/** the ip address of the client */
extern char client_ip[20];

void create_requests_logs_file(void);

void create_errors_logs_file(void);

void write_request(FILE *log_file, http_request request, int code);

void write_error(FILE *log_file, char *error);

FILE *get_log_requests(void);

FILE *get_log_errors(void);

struct tm *get_actual_time(void);

#endif