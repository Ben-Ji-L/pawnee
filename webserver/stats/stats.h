# ifndef __STATS_H__
# define __STATS_H__

#include "../http/http_parse.h"

/** struct for the stats */
typedef struct {

    /** number of connections the server has received */
    int served_connections;

    /** number of request the server has received */
    int served_requests;

    /** number of 200 responses the server has sent */
    int ok_200;

    /** number of 400 responses the server has sent */
    int ko_400;

    /** number of 403 responses the server has sent */
    int ko_403;

    /** number of 404 responses the server has sent */
    int ko_404;

    /** number of 405 responses the server has sent */
    int ko_405;
} web_stats;

/** semaphore to avoid concurrent access to the stats */
extern sem_t *shared_semaphore;

void send_stats(FILE *client, http_request *request);

void init_stats(void);

web_stats *get_stats(void);

#endif