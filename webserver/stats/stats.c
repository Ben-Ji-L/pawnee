#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "stats.h"
#include "../http/http.h"
#include "../log/log.h"

static web_stats stats;

/** shared memory for the stats */
web_stats *shared_memory;
sem_t *shared_semaphore;

/**
 * function who display stats of the server in html
 * @param client the socket of the client
 */
void send_stats(FILE *client) {
    send_status(client, 200, "OK");
    fprintf(client, "Content-Length: %d\r\n", 484);
    fprintf(client, "Content-Type: %s\r\n", "text/html");
    fprintf(client, "Date: %s\r\n", get_date_http_format());
    fprintf(client, "\r\n");
    fprintf(client, "<!DOCTYPE html><html>\
        <head><meta charset=\"UTF-8\">\
        <style>body {font: 1.2em \"Open Sans\", sans-serif; background-color: #DDDDDD;}</style>\
        </head>\
        <body>\
            <h1>Stats of the server</h1><ul><li>Connexions served: %d</li> \
            <li>Requests served: %d</li> \
            <li>200 responses: %d</li> \
            <li>400 responses: %d</li> \
            <li>403 responses: %d</li> \
            <li>404 responses: %d</li> \
            <li>405 responses: %d</li></ul>" \
       "</body></html>\r\n", get_stats()->served_connections,
                get_stats()->served_requests, get_stats()->ok_200, get_stats()->ko_400, get_stats()->ko_403,
                get_stats()->ko_404, get_stats()->ko_405);
}

/**
 * init the server stats, the semaphore and the shared memory
 */
void init_stats(void) {

    // init of the semaphore
    sem_t semaphore;
    if (sem_init(&semaphore, 0, 1) == -1) {
        write_error(get_log_errors(), "sem_init error");
        exit(EXIT_FAILURE);
    }

    // init of the shared memory zone
    shared_memory = mmap(NULL, sizeof(stats), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED) {
        write_error(get_log_errors(), "mmap stats error");
        exit(EXIT_FAILURE);
    }
    memcpy(shared_memory, &stats, sizeof(stats));

    // init the shared memory zone for the semaphore
    shared_semaphore = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_semaphore == MAP_FAILED) {
        write_error(get_log_errors(), "mmap semaphore error");
        exit(EXIT_FAILURE);
    }
    memcpy(shared_semaphore, &semaphore, sizeof(semaphore));

    // init the stats to 0
    stats.served_connections = 0;
    stats.served_requests = 0;
    stats.ok_200 = 0;
    stats.ko_400 = 0;
    stats.ko_403 = 0;
    stats.ko_404 = 0;
    stats.ko_405 = 0;
}

/**
 * return the shared memory zone of the stats
 * @return a pointer to the server stats
 */
web_stats *get_stats(void) {
    return shared_memory;
}