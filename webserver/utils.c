#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "http_parse.h"

char *fgets_or_exit(char *buffer, int size, FILE *stream) {
	if (fgets(buffer, size, stream) == NULL) {
		perror("fgets error");
		exit(1);
	}
	return buffer;
}

void skip_headers(FILE *client) {

	char data[512];
	do {
		fgets_or_exit(data, 512, client);
    } while (strncmp(data, "\r\n", 2) != 0);
}

void send_status(FILE *client, int code, const char *reason_phrase) {
	fprintf(client, "HTTP/1.1 %d %s\r\n", code, reason_phrase);
}

void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body) {
	send_status(client, code, reason_phrase);
	fprintf(client, "Content-size: %d\r\n", (int) strlen(message_body));
	fprintf(client, "\r\n");
	fprintf(client, "%s", message_body);
}