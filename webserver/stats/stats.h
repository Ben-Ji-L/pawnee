# ifndef __STATS_H__
# define __STATS_H__

/* Structure contenant les statistiques */
typedef struct {
    int served_connections;
    int served_requests;
    int ok_200;
    int ko_400;
    int ko_403;
    int ko_404;
    int ko_405;
} web_stats;

// Le sémaphore pour éviter les accés concurrents aux statistiques
sem_t *shared_semaphore;

void send_stats(FILE *client);

void init_stats(void);

web_stats *get_stats(void);

#endif