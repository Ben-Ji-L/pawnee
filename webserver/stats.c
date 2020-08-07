#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "stats.h"
#include "utils.h"

static web_stats stats;

void send_stats(FILE *client) {
    send_status(client, 200, "OK");
    fprintf(client, "Content-Length: %d\r\n", 325);
    fprintf(client, "Content-Type: %s\r\n", "text/html");
	fprintf(client, "\r\n");
	fprintf(client, "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><h1>Statistiques</h1><ul><li>Connections servies : %d</li> \
        <li>Requetes servies : %d</li> \
        <li>Réponses 200 : %d</li> \
        <li>Réponses 400 : %d</li> \
        <li>Réponses 403 : %d</li> \
        <li>Réponses 404 : %d</li></ul></body></html>\r\n", stats.served_connections, stats.served_requests, stats.ok_200, stats.ko_400, stats.ko_403, stats.ko_404);
}

int init_stats(void) {
    stats.served_connections = 0;
    stats.served_requests = 0;
    stats.ok_200 = 0;
    stats.ko_400 = 0;
    stats.ko_403 = 0;
    stats.ko_404 = 0;

    return 0;
}

web_stats *get_stats(void) {
    return &stats;
}