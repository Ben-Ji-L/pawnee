# ifndef __STATS_H__
# define __STATS_H__

typedef struct {
    int served_connections;
    int served_requests;
    int ok_200;
    int ko_400;
    int ko_403;
    int ko_404;
} web_stats;

sem_t *shared_semaphore;

void send_stats(FILE *client);
int init_stats(void);
web_stats *get_stats(void);

#endif