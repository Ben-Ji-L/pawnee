#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/limits.h>
#include <semaphore.h>

#include "socket.h"
#include "utils.h"
#include "http_parse.h"
#include "stats.h"
#include "config.h"

void initialiser_signaux(void);
void repondre_client(int socket_client);
void child_handler(void);

char *root;


int main(int argc, char *argv[]) {
    get_app_path(argv[0]);
    init_config();

    if (argc > 1) {
        root = check_root(argv[1]);
        argc++;
    } else
        root = check_root(get_config()->website_root);

    // Les deux sockets dont on aura besoin.
    int socket_serveur;
    int socket_client;

    // On crée le serveur
    socket_serveur = creer_serveur(get_config()->port);

    printf("serveur lancé\n");

    if (listen(socket_serveur, 10) == -1){
        perror("error socket_serveur");
        exit(1);
    }

    initialiser_signaux();
    init_stats();

    // Le serveur attend des requetes
    while(1) {
        // On accepte une connexion
        socket_client = accept(socket_serveur, NULL, NULL);
        if (socket_client == -1) {
            perror("accept error");
            exit(1);
        }

        sem_wait(shared_semaphore);
        get_stats()->served_connections++;
        sem_post(shared_semaphore);

        // Si pas d'erreur on répond au client
        repondre_client(socket_client);

        // On ferme le socket
        close(socket_client);
    }

    return 0;
}

/**
 * Cette fonction sert à ignorer le signal SIGPIPE
 * ce signal est déclenché lorsque le programme essaie d'écrire à travers le socket
 * et que celui ci est déconnecté.
 */
void initialiser_signaux(void) {

    // On ignore le signal SIGPIPE
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal error");
        exit(1);
    }

    // Traitement signal SIGCHLD
    struct sigaction sa;

    sa.sa_handler = (__sighandler_t) child_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD,  &sa, NULL) == -1) {
        perror("error sigaction");
        exit(1);
    }


}

/**
 * Cette fonction permet de faire disparaitre les processus zombies.
 */
void child_handler(void) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

/**
 * La fonction qui s'occupe de toute la logique de la réponse.
 * @param socket_client le socket à travers lequel il faut envoyer la réponse
 */
void repondre_client(int socket_client) {

    // On crée un nouveau processus à chaque requete pour pouvoir traiter plusieurs client àla fois.
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork error");
        exit(1);
    }

    // Si on est dans un processus fils.
    if (pid == 0) {
        FILE *flux;
        FILE *file;
        char data[512];

        // On ouvre le socket en lecture et en écriture
        flux = fdopen(socket_client, "w+");
        fgets_or_exit(data, 512, flux);
        skip_headers(flux);
        
        http_request request;

        sem_wait(shared_semaphore);
        get_stats()->served_requests++;
        sem_post(shared_semaphore);
        
        // Ici on parse la requete et selon l'en-tete on envoie la réponse appropriée.
        if (!parse_http_request(data, &request)) {
            // En cas de requete mal écrite.
            sem_wait(shared_semaphore);
            get_stats()->ko_400++;
            sem_post(shared_semaphore);

        	send_response(flux, 400, "Bad Request", "Bad request", strlen("Bad request\r\n"));
        } else if (request.method == HTTP_UNSUPPORTED) {
            // Si la méthode n'est pas supportée par le serveur sur cette URL.
        	send_response(flux, 405, "Method Not Allowed", "Method Not Allowed", strlen("Method Not Allowed\r\n"));
        } else {

            if (strcmp(rewrite_target(request.target), "stats") == 0) {
                send_stats(flux);
                fclose(flux);
                exit(0);
            }

            file = check_and_open(rewrite_target(request.target), root);
		    if (file == NULL) {
                sem_wait(shared_semaphore);
                get_stats()->ko_404++;
                sem_post(shared_semaphore);

			    send_response(flux, 404, "Not Found", "Not Found", strlen("Not Found\r\n"));
                fclose(flux);
                exit(0);
            } else {
                sem_wait(shared_semaphore);
                get_stats()->ok_200++;
                sem_post(shared_semaphore);

                send_response(flux, 200, "OK", rewrite_target(request.target), get_file_size(fileno(file)));
                copy(file, flux);
            }
            fclose(file);
        }
        // On oublie pas de fermer le socket
        fclose(flux);
        exit(0);
    }
}
