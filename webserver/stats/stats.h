# ifndef __STATS_H__
# define __STATS_H__

/**
 * Structure contenant les statistiques
 */
typedef struct {

    /**
     * le nombres de connexions que le serveur à recues
     */
    int served_connections;

    /**
     * le nombres de requêtes que le serveur à recues
     */
    int served_requests;

    /**
     * le nombre de réponses 200 que le serveur à envoyer
     */
    int ok_200;

    /**
     * le nombre de réponses 400 que le serveur à envoyer
     */
    int ko_400;

    /**
     * le nombre de réponses 403 que le serveur à envoyer
     */
    int ko_403;

    /**
     * le nombre de réponses 404 que le serveur à envoyer
     */
    int ko_404;

    /**
     * le nombre de réponses 405 que le serveur à envoyer
     */
    int ko_405;
} web_stats;

// Le sémaphore pour éviter les accés concurrents aux statistiques
sem_t *shared_semaphore;

void send_stats(FILE *client);

void init_stats(void);

web_stats *get_stats(void);

#endif