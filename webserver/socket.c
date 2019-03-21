#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>

#include "socket.h"

/**
 * On met en place le serveur dans cette fonction en précisant le port que l'on souhaite écouter.
 * @param port Le port sur lequel on souhaite que le serveur écoute.
 * @return Le socket du serveur.
 */
int creer_serveur(int port){

    int socket_serveur;
    int optval = 1;

    // On crée la socket pour que le serveur puisse écouter le réseau.
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

    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        perror("option SO_REUSEADDR error");
        return -1;
    }

    if (bind(socket_serveur, (struct sockaddr*)&saddr , sizeof(saddr)) == -1){
        perror("bind socket_serveur");/* traitement de l ’ erreur */
        return -1;
    }

    return socket_serveur;
}