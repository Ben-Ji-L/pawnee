#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socket.h"
#include "config/config.h"
#include "log.h"

/**
 * creation of the server with the port we want to listen
 * @param port the port to listen
 * @return the server socket
 */
int create_server(int port) {

    int socket_server;
    int optval = 1;

    /* creation of the socket for listening the network */
    socket_server = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_server == -1) {
        /* processing error */
        write_error(get_log_errors(), "socket_server ");
        return -1;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET; /* Socket ipv4 */
    saddr.sin_port = htons(port); /* port to listen */
    saddr.sin_addr.s_addr = inet_addr(get_config()->listen_addr); /* listen on all interfaces */

    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        write_error(get_log_errors(), "option SO_REUSEADDR error");
        return -1;
    }

    if (bind(socket_server, (struct sockaddr *) &saddr, sizeof(saddr)) == -1) {
        write_error(get_log_errors(), "bind socket_server");/* processing error */
        return -1;
    }

    return socket_server;
}