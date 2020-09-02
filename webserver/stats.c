#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "stats.h"
#include "utils.h"
#include "http.h"

static web_stats stats;
web_stats *shared_memory;

/**
 * Fonction qui affiche les statistiques au format html
 * @param client le socket à qui on envoi les données
 */
void send_stats(FILE *client) {
    send_status(client, 200, "OK");
    fprintf(client, "Content-Length: %d\r\n", 325);
    fprintf(client, "Content-Type: %s\r\n", "text/html");
	fprintf(client, "\r\n");
	fprintf(client, "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><h1>Statistiques</h1><ul><li>Connections servies : %d</li> \
        <li>Requêtes servies : %d</li> \
        <li>Réponses 200 : %d</li> \
        <li>Réponses 400 : %d</li> \
        <li>Réponses 403 : %d</li> \
        <li>Réponses 404 : %d</li></ul></body></html>\r\n", get_stats()->served_connections, get_stats()->served_requests, get_stats()->ok_200, get_stats()->ko_400, get_stats()->ko_403, get_stats()->ko_404);
}

/**
 * Initialise les statistiques, le sémaphore et la mémoire partagée
 */
void init_stats(void) {
    // On créer le sémaphore
    sem_t semaphore;
    if (sem_init(&semaphore, 0, 1) == -1) {
        perror("sem_init error");
        exit(EXIT_FAILURE);
    }

    // On initialise la zone mémoire partagée
    shared_memory = mmap(NULL, sizeof(stats), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap stats error");
        exit(EXIT_FAILURE);
    }
    memcpy(shared_memory, &stats, sizeof(stats));

    // On crée une mémoire partagée pour le sémaphore
    shared_semaphore = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_semaphore == MAP_FAILED) {
        perror("mmap semaphore error");
        exit(EXIT_FAILURE);
    }
    memcpy(shared_semaphore, &semaphore, sizeof(semaphore));

    // On initialise les statistiques à 0
    stats.served_connections = 0;
    stats.served_requests = 0;
    stats.ok_200 = 0;
    stats.ko_400 = 0;
    stats.ko_403 = 0;
    stats.ko_404 = 0;
}

/**
 * Renvoie la zone mémoire partagée permettant l'accés aux statistiques
 * @return un pointeur vers les statistiques
 */
web_stats *get_stats(void) {
    return shared_memory;
}