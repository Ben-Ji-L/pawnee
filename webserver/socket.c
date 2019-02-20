#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>

int creer_serveur(int port){

	int socket_serveur;
	int optval = 1;

	socket_serveur = socket(AF_INET, SOCK_STREAM, 0);

 	if (socket_serveur == -1){
		/* traitement de l ’ erreur */
		perror("socket_serveur ");
		return -1;
	}

	struct sockaddr_in saddr ;
	saddr.sin_family = AF_INET ; /* Socket ipv4 */
	saddr.sin_port = htons(port); /* Port d ’é coute */
	saddr.sin_addr.s_addr = INADDR_ANY ; /* é coute sur toutes les interfaces */

	if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
	    perror("option SO_REUSEADDR error");

	if (bind (socket_serveur, (struct sockaddr*)&saddr , sizeof(saddr)) == -1){
		perror("bind socker_serveur");/* traitement de l ’ erreur */
		return -1;
	}

	return socket_serveur;
}