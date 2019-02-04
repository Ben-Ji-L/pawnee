#include <stdio.h>
#include <string.h>
#include "socket.h"

int main(int argc, char **argv)
{

	creer_serveur(8080);

	if (listen ( socket_serveur , 10) == -1){
		perror ("listen socket_serveur");
		exit(1);
	}

	return 0;
}