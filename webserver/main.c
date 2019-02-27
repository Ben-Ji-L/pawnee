#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "socket.h"

void initialiser_signaux(void);

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

	socket_client = accept(socket_serveur, NULL, NULL);

	if (socket_client == -1) {
		perror("accept error");
		exit(1);
	}
    initialiser_signaux();
	sleep(1);
	printf("Ca marche\n");
	const char *message = "Mais, vous savez, moi je ne crois pas\n qu\'il y ait de bonne ou de mauvaise situation.\nMoi, si je devais resumer ma vie aujourd\'hui avec vous,\nje dirais que c\'est d\'abord des rencontres,\nDes gens qui m\'ont tendu la main,\npeut-etre a un moment ou je ne pouvais pas, ou j\'étais seul chez moi.\nEt c\'est assez curieux de se dire que les hasards,\nles rencontres forgent une destinee...\nParce que quand on a le gout de la chose,\nquand on a le gout de la chose bien faite,\nLe beau geste, parfois on ne trouve pas l\'interlocuteur en face,\nje dirais, le miroir qui vous aide a avancer.\nAlors ce n\'est pas mon cas, comme je le disais la,\npuisque moi au contraire, j\'ai pu ;\nEt je dis merci a la vie, je lui dis merci,\nje chante la vie, je danse la vie... Je ne suis qu\'amour!\nEt finalement, quand beaucoup de gens aujourd\'hui me disent :\n\"Mais comment fais-tu pour avoir cette humanite \?\",\nEh bien je leur reponds tres simplement,\nje leur dis que c\'est ce gout de l\'amour,\nCe gout donc qui m\'a pousse aujourd\'hui\na entreprendre une construction mecanique,\nMais demain, qui sait, peut-etre simplement\na me mettre au service de la communaute,\na faire le don, le don de soi...\n\n";
    if (write(socket_client, message, strlen(message)) == -1) {
			perror("write boucle for error");
			exit(1);
		}

	int i;
	for (i = 0; i < 10; i++)
    {
        if (write(socket_client, message, strlen(message)) == -1) {
			perror("write boucle for error");
			exit(1);
		}

		sleep(1);
	}

	while(1) {
		char message_client[512];
		ssize_t taille = read(socket_client, message_client, 512);
		if (taille == -1)
		{
			perror("read boucle error");
			exit(1);
		}

        write(socket_client, message_client, (size_t) taille);
	}

	return 0;
}

void initialiser_signaux(void) {
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal error");
        exit(1);
    }
}