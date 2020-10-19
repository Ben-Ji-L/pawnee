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

    char executable_path[PATH_MAX];
    strncpy(executable_path, get_app_path(), PATH_MAX);

    if (init_config(executable_path) != 0) {
        perror("init config error");
        exit(1);
    }

    create_requests_logs_file(executable_path);
    create_errors_logs_file(executable_path);

    if (argc > 1) {
        strncpy(root, check_root(argv[1]), PATH_MAX);
        argc++;
    }

    /* the two sockets that we need */
    int socket_server;
    int socket_client;

    /* creation of the server */
    socket_server = create_server(get_config()->port);

    printf("server running at : http://%s:%d\n", get_config()->listen_addr, get_config()->port);

    if (listen(socket_server, 10) == -1) {
        write_error(get_log_errors(), "error socket_server");
        exit(1);
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
            exit(1);
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

    fclose(get_log_requests());
    fclose(get_log_errors());

    return 0;
}

/**
 * function to ignore SIGPIPE signal
 */
void init_signals(void) {

    /* we ignore the SIGPIPE signal */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        write_error(get_log_errors(), "signal error");
        exit(1);
    }

    /* processing the SIGCHLD signal */
    struct sigaction sa;

    sa.sa_handler = (__sighandler_t) child_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        write_error(get_log_errors(), "error sigaction");
        exit(1);
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

    /* we create a new process for each requests */
    pid_t pid = fork();

    if (pid == -1) {
        write_error(get_log_errors(), "fork error");
        exit(1);
    }

    /* if this is the son process */
    if (pid == 0) {
        FILE *flux;
        FILE *file;
        char data[512];
        http_request request;

        /* we open the socket to read and to write */
        flux = fdopen(socket_client, "w+");
        if (flux == NULL) {
            write_error(get_log_errors(), "error socket client");
            exit(1);
        }

        if (fgets_or_exit(data, 512, flux) == NULL) {
            write_error(get_log_errors(), "read file request error");
            exit(EXIT_FAILURE);
        }

        skip_and_save_headers(flux, &request);
        int host_check = check_host_header(&request);

        sem_wait(shared_semaphore);
        get_stats()->served_requests++;
        sem_post(shared_semaphore);

        /* parsing request and sending appropriate response */
        if ((!parse_http_request(data, &request) && (request.method != HTTP_UNSUPPORTED)) || (host_check != 0)) {
            /* if the requests is miss written */
            write_request(get_log_requests(), request, 400);

            sem_wait(shared_semaphore);
            get_stats()->ko_400++;
            sem_post(shared_semaphore);

            send_response(flux, 400, "Bad Request", "Bad request", strlen("Bad request\r\n"));
        } else if (request.method == HTTP_UNSUPPORTED) {
            write_request(get_log_requests(), request, 405);

            sem_wait(shared_semaphore);
            get_stats()->ko_405++;
            sem_post(shared_semaphore);

            /* if the method is unsupported */
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

            char *host = get_vhost_root(&request);
            if (host == NULL) {
                write_error(get_log_errors(), "vhost root error");
                exit(EXIT_FAILURE);
            }
            strcpy(root, check_root(host));

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

                /* if client send request with HEAD method */
                if (request.method == HTTP_HEAD) {
                    fclose(flux);
                    fclose(file);
                    exit(0);
                }

                copy(file, flux);
            }
            fclose(file);
        }
        /* closing socket */
        fclose(flux);
        exit(0);
    }
}
