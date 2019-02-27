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
void child_handler(int signal);

int main()
{
	int socket_serveur;
	int socket_client;
	socket_serveur = creer_serveur(8080);

	printf("serveur cree\n");

	if (listen(socket_serveur, 10) == -1){
		perror("error socket_serveur");
		exit(1);
	}

	initialiser_signaux();
	sleep(1);
	printf("Ca marche\n");

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

   	sa.sa_handler = child_handler;
   	sigemptyset(&sa.sa_mask);
   	sa.sa_flags = SA_RESTART;

   	if (sigaction(SIGCHLD,  &sa, NULL) == -1) {
   		perror("error sigaction");
   		exit(1);
   	}


}

void child_handler(int signal) {
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

		fgets(data, 512, flux);

		fprintf(flux, "%s", data);
		fclose(flux);
		exit(0);
	}
}
