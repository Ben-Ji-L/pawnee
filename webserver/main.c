#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "socket.h"

void initialiser_signaux(void);
void repondre_client(int socket_client);
void child_handler(void);

int main() {
    int socket_serveur;
    int socket_client;
    socket_serveur = creer_serveur(8080);

    printf("serveur cree\n");

    if (listen(socket_serveur, 10) == -1){
        perror("error socket_serveur");
        exit(1);
    }

    initialiser_signaux();

    while(1) {
        socket_client = accept(socket_serveur, NULL, NULL);
        if (socket_client == -1) {
            perror("accept error");
            exit(1);
        }
        repondre_client(socket_client);
        close(socket_client);
    }
    return 0;
}

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

void child_handler(void) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

void repondre_client(int socket_client) {

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork error");
        exit(1);
    }

    if (pid == 0) {
        FILE* flux;
        char data[512];

        flux = fdopen(socket_client, "w+");

        fgets(data, 100, flux);
        printf("%s", data);

        if (strncmp(data, "GET /inexistant HTTP/1.1\r\n", 100) == 0) {

            do {
                fgets(data, 512, flux);
            } while (strncmp(data, "\r\n", 2) != 0);

            fprintf(flux, "HTTP/1.1 404 Not Found\r\n"
                          "Connection: Keep-Alive\r\n"
                          "Content-Length: 0\r\n"
                          "404 Not Found\r\n\r\n");

            fclose(flux);
            exit(0);

        } else if (strncmp(data, "GET / HTTP/1.1\r\n", 100) != 0) {

            do {
                fgets(data, 512, flux);
            } while (strncmp(data, "\r\n", 2) != 0);

            fprintf(flux, "HTTP/1.1 400 Bad Request\r\n"
                          "Connection: close\r\n"
                          "Content-Length: 0\r\n"
                          "400 Bad Request\r\n\r\n");

            fclose(flux);
            exit(0);
        }

        do {
            fgets(data, 512, flux);
        } while (strncmp(data, "\r\n", 2) != 0);

        char* bienvenue = "<style>"
                          "h1 { font-family: Montserrat; font-weight: bold; color: purple;}"
                          "</style>"
                          "<h1>Bonjour et bienvenue sur notre serveur</h1\r\n";
        fprintf(flux, "HTTP/1.1 200 OK\r\n"
                      "Connection: open\r\n"
                      "Content-Length: %d\r\n"
                      "200 OK\r\n\r\n", (int) strlen(bienvenue));

        fprintf(flux, "%s", bienvenue);
        fclose(flux);
        exit(0);
    }
}
