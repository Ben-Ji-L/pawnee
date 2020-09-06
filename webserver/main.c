#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <semaphore.h>
#include <arpa/inet.h>

#include "socket.h"
#include "http_parse.h"
#include "stats.h"
#include "config.h"
#include "http.h"
#include "file.h"
#include "log.h"

void initialiser_signaux(void);
void repondre_client(int socket_client);
void child_handler(void);

char root[PATH_MAX];

int main(int argc, char *argv[]) {

    char executable_path[PATH_MAX];
    strncpy(executable_path, get_app_path(argv[0]), PATH_MAX);

    init_config(executable_path);

    create_requests_logs_file(executable_path);
    create_errors_logs_file(executable_path);

    if (argc > 1) {
        strncpy(root, check_root(argv[1]), PATH_MAX);
        argc++;
    } else
        strncpy(root, check_root(get_config()->website_root), PATH_MAX);

    // Les deux sockets dont on aura besoin.
    int socket_serveur;
    int socket_client;

    // On crée le serveur
    socket_serveur = creer_serveur(get_config()->port);

    printf("serveur lancé à l'adresse : http://%s:%d\n", get_config()->listen_addr, get_config()->port);

    if (listen(socket_serveur, 10) == -1){
        write_error(get_log_errors(), "error socket_serveur");
        exit(1);
    }

    initialiser_signaux();
    init_stats();

    // Le serveur attend des requêtes
    while(1) {

        // Les variables néccessaires pour trouver l'adresse ip du client
        struct sockaddr_in addr_client_struct;
        socklen_t clientaddr_size = sizeof(addr_client_struct);
        
        // On accepte une connexion
        socket_client = accept(socket_serveur, (struct sockaddr *) &addr_client_struct, &clientaddr_size);
        if (socket_client == -1) {
            write_error(get_log_errors(), "accept error");
            exit(1);
        }

        // Détermination de l'adresse ip du client
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        if (getpeername(socket_client, (struct sockaddr *)&addr, &addr_size) == -1) {
            write_error(get_log_errors(), "getpeername error");
            exit(EXIT_FAILURE);
        }
        strncpy(clientip, inet_ntoa(addr.sin_addr), 20);

        sem_wait(shared_semaphore);
        get_stats()->served_connections++;
        sem_post(shared_semaphore);

        // Si pas d'erreur on répond au client
        repondre_client(socket_client);

        // On ferme le socket
        close(socket_client);
    }

    fclose(get_log_requests());
    fclose(get_log_errors());

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
        write_error(get_log_errors(), "signal error");
        exit(1);
    }

    // Traitement signal SIGCHLD
    struct sigaction sa;

    sa.sa_handler = (__sighandler_t) child_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD,  &sa, NULL) == -1) {
        write_error(get_log_errors(), "error sigaction");
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
        write_error(get_log_errors(), "fork error");
        exit(1);
    }

    // Si on est dans un processus fils.
    if (pid == 0) {
        FILE *flux;
        FILE *file;
        char data[512];

        // On ouvre le socket en lecture et en écriture
        flux = fdopen(socket_client, "w+");
        if (flux == NULL) {
            write_error(get_log_errors(), "error socket client");
            exit(1);
        }
        
        fgets_or_exit(data, 512, flux);
        skip_headers(flux);
        
        http_request request;

        sem_wait(shared_semaphore);
        get_stats()->served_requests++;
        sem_post(shared_semaphore);

        // Ici on parse la requete et selon l'en-tete on envoie la réponse appropriée.
        if (!parse_http_request(data, &request)) {
            // En cas de requete mal écrite.
            write_request(get_log_requests(), request, 400);

            sem_wait(shared_semaphore);
            get_stats()->ko_400++;
            sem_post(shared_semaphore);

        	send_response(flux, 400, "Bad Request", "Bad request", strlen("Bad request\r\n"));
        } else if (request.method == HTTP_UNSUPPORTED) {
            write_request(get_log_requests(), request, 405);

            // Si la méthode n'est pas supportée par le serveur sur cette URL.
        	send_response(flux, 405, "Method Not Allowed", "Method Not Allowed", strlen("Method Not Allowed\r\n"));
        } else {

            if (strcmp(rewrite_target(request.target), "stats") == 0) {
                write_request(get_log_requests(), request, 200);

                sem_wait(shared_semaphore);
                get_stats()->ok_200++;
                sem_post(shared_semaphore);

                send_stats(flux);
                fclose(flux);
                exit(0);
            }

            file = check_and_open(rewrite_target(request.target), root);
		    if (file == NULL) {
                write_request(get_log_requests(), request, 404);

                sem_wait(shared_semaphore);
                get_stats()->ko_404++;
                sem_post(shared_semaphore);

			    send_response(flux, 404, "Not Found", "Not Found", strlen("Not Found\r\n"));
                fclose(flux);
                exit(0);
            } else {
                write_request(get_log_requests(), request, 200);

                sem_wait(shared_semaphore);
                get_stats()->ok_200++;
                sem_post(shared_semaphore);

                send_response(flux, 200, "OK", rewrite_target(request.target), get_file_size(fileno(file)));

                // si le client envoie une requête avec la méthode HEAD
                if (request.method == HTTP_HEAD) {
                    fclose(flux);
                    fclose(file);
                    exit(0);
                }

                copy(file, flux);
            }
            fclose(file);
        }
        // On oublie pas de fermer le socket
        fclose(flux);
        exit(0);
    }
}
