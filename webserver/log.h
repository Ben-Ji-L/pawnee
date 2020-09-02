# ifndef __LOG_H__
#define __LOG_H__

FILE *create_requests_logs_file(char *path);
FILE *create_errors_logs_file(char *path);

void write_request(FILE *log_file, http_request request, int code);

#endif