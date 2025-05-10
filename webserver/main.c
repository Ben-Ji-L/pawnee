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
#include "http/http_parse.h"
#include "stats/stats.h"
#include "config/config.h"
#include "http/http.h"
#include "file.h"
#include "log/log.h"
#include "vhosts/hosts.h"

void init_signals(void);

void respond_client(int socket_client);

void child_handler(void);

/** the root directory of the application */
char root[PATH_MAX];

/**
 * the main function of the program
 * @param argc arguments counter
 * @param argv arguments array
 * @return 0 on success, positive value on error
 */
int main(int argc, char *argv[]) {
    if (init_config() != 0) {
        perror("init config error");
        exit(EXIT_FAILURE);
    }

    create_requests_logs_file();
    create_errors_logs_file();

    if (argc > 1)
        strncpy(root, check_root(argv[1]), PATH_MAX);

    /* the two sockets that we need */
    int socket_server;
    int socket_client;

    /* creation of the server */
    socket_server = create_server(get_config()->port);

    printf("server running at : http://%s:%d\n", get_config()->listen_addr, get_config()->port);

    if (listen(socket_server, 10) == -1) {
        write_error(get_log_errors(), "error socket_server");
        exit(EXIT_FAILURE);
    }

    init_signals();
    init_stats();

    /* server waiting requests */
    while (1) {

        /* vars needed to find ip address of the client */
        struct sockaddr_in addr_client_struct;
        socklen_t client_addr_size = sizeof(addr_client_struct);

        /* we accept a connection */
        socket_client = accept(socket_server, (struct sockaddr *) &addr_client_struct, &client_addr_size);
        if (socket_client == -1) {
            write_error(get_log_errors(), "accept error");
            exit(EXIT_FAILURE);
        }

        /* finding the ip address of the client */
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        if (getpeername(socket_client, (struct sockaddr *) &addr, &addr_size) == -1) {
            write_error(get_log_errors(), "getpeername error");
            exit(EXIT_FAILURE);
        }
        strncpy(client_ip, inet_ntoa(addr.sin_addr), 20);

        sem_wait(shared_semaphore);
        get_stats()->served_connections++;
        sem_post(shared_semaphore);

        /* if no errors, we send response to the client */
        respond_client(socket_client);

        /* we close the socket */
        close(socket_client);
    }
}

/**
 * function to ignore SIGPIPE signal
 */
void init_signals(void) {

    /* we ignore the SIGPIPE signal */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        write_error(get_log_errors(), "signal error");
        exit(EXIT_FAILURE);
    }

    /* processing the SIGCHLD signal */
    struct sigaction sa;

    sa.sa_handler = (__sighandler_t) child_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        write_error(get_log_errors(), "error sigaction");
        exit(EXIT_FAILURE);
    }


}

/**
 * function to dismiss zombies process
 */
void child_handler(void) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

/**
 * function to respond to the client
 * @param socket_client the socket client
 */
void respond_client(int socket_client) {
    pid_t pid = fork();

    if (pid == -1) {
        write_error(get_log_errors(), "fork error");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // processus fils
        FILE *flux = fdopen(socket_client, "w+");
        if (!flux) {
            write_error(get_log_errors(), "fdopen error on socket");
            exit(EXIT_FAILURE);
        }

        if (flux == NULL) {
            write_error(get_log_errors(), "fdopen error, client null");
            exit(EXIT_FAILURE);
        }

        char request_line[1024], data[1024], *error_message = "";
        http_request request;

        if (!fgets(request_line, sizeof(request_line), flux) ||
            !fgets(data, sizeof(data), flux)) {
            write_error(get_log_errors(), "error reading from client");
            fclose(flux);
            exit(EXIT_FAILURE);
        }

        sem_wait(shared_semaphore);
        get_stats()->served_requests++;
        sem_post(shared_semaphore);

        if (!parse_http_request(request_line, data, &request)) {
            write_request(get_log_requests(), request, 400);
            sem_wait(shared_semaphore);
            get_stats()->ko_400++;
            sem_post(shared_semaphore);

            if (request.method != HTTP_HEAD)
                error_message = "Bad request\r\n";
            send_response(flux, &request, 400, "Bad Request", error_message, strlen(error_message));
            fclose(flux);
            exit(EXIT_SUCCESS);
        }

        if (request.method == HTTP_UNSUPPORTED) {
            write_request(get_log_requests(), request, 405);
            sem_wait(shared_semaphore);
            get_stats()->ko_405++;
            sem_post(shared_semaphore);

            if (request.method != HTTP_HEAD)
                error_message = "Method Not Allowed\r\n";
            send_response(flux, &request, 405, "Method Not Allowed", error_message, strlen(error_message));
            fclose(flux);
            exit(EXIT_SUCCESS);
        }

        if (check_http_version(&request) != 0) {
            request.http_major = 1;
            request.http_minor = 1;

            if (request.method != HTTP_HEAD)
                error_message = "HTTP Version Not Supported\r\n";
            send_response(flux, &request, 505, "HTTP Version Not Supported", error_message, strlen(error_message));
            fclose(flux);
            exit(EXIT_SUCCESS);
        }

        const char *target = rewrite_target(request.target);
        if (strcmp(target, "stats") == 0) {
            write_request(get_log_requests(), request, 200);
            sem_wait(shared_semaphore);
            get_stats()->ok_200++;
            sem_post(shared_semaphore);

            send_stats(flux, &request);
            fclose(flux);
            exit(EXIT_SUCCESS);
        }

        char *host_root = get_vhost_root(&request);

        if (host_root == NULL) {
            write_error(get_log_errors(), "vhost root error");
            write_request(get_log_requests(), request, 400);
            sem_wait(shared_semaphore);
            get_stats()->ko_400++;
            sem_post(shared_semaphore);

            if (request.method != HTTP_HEAD)
                error_message = "Bad request\r\n";
            send_response(flux, &request, 400, "Bad Request", error_message, strlen(error_message));
            fclose(flux);
            exit(EXIT_SUCCESS);
        }

        char *root = check_root(host_root);

        FILE *file = check_and_open(target, root);
        if (!file) {
            write_request(get_log_requests(), request, 404);
            sem_wait(shared_semaphore);
            get_stats()->ko_404++;
            sem_post(shared_semaphore);

            if (request.method != HTTP_HEAD)
                error_message = "Not Found\r\n";
            send_response(flux, &request, 404, "Not Found", error_message, strlen(error_message));
            fclose(flux);
            exit(EXIT_SUCCESS);
        }

        write_request(get_log_requests(), request, 200);
        sem_wait(shared_semaphore);
        get_stats()->ok_200++;
        sem_post(shared_semaphore);
        
        send_response(flux, &request, 200, "OK", rewrite_target(request.target), get_file_size(fileno(file)));
        fclose(flux);
        exit(EXIT_SUCCESS);
    }
}
