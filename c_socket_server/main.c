#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    struct addrinfo addrinfo_hints;
    memset(&addrinfo_hints, 0, sizeof(addrinfo_hints));

    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        addrinfo_hints.ai_family   = AF_UNSPEC;
        addrinfo_hints.ai_socktype = SOCK_STREAM;
        addrinfo_hints.ai_flags    = AI_PASSIVE;

        struct addrinfo* addrinfo_result;
        if (getaddrinfo(
            NULL, "8080",
            &addrinfo_hints, &addrinfo_result) != 0
        ) {
            perror("getaddrinfo");
            exit(1);
        }

        int listen_sock_fd = -1;
        bool bound_socket = false;
        for (struct addrinfo* p = addrinfo_result;
             p != NULL;
             p = p->ai_next
        ) {
            listen_sock_fd = socket(p->ai_family,
                                    p->ai_socktype,
                                    p->ai_protocol);
            if (listen_sock_fd == -1)
                continue;

            if (bind(listen_sock_fd,
                     p->ai_addr,
                     p->ai_addrlen) == -1
            ) {
                close(listen_sock_fd);
                continue;
            }

            bound_socket = true;
            break;
        }
        freeaddrinfo(addrinfo_result);
        if (!bound_socket) {
            perror("Failed to bind IPv4 and IPv6");
            exit(1);
        }

        if (listen(listen_sock_fd, 10) == -1) {
            perror("listen");
            exit(1);
        }

        struct sockaddr_storage conn_addr;
        socklen_t               conn_addr_sz = sizeof(conn_addr_sz);
        while (true) {
            int conn_sock_fd = accept(
                listen_sock_fd,
                (struct sockaddr*) &conn_addr,
                &conn_addr_sz);
            if (conn_sock_fd == -1) {
                perror("accept");
                continue;
            }

            while (true) {
                char cmd[1024];
                int n = recv(conn_sock_fd, cmd, 1024, 0);
                if (n == -1) {
                    perror("recv");
                    goto server_conn_cleanup;
                }
                cmd[n] = '\0';
                if (strcmp(cmd, "ping") == 0) {
                    if (send(conn_sock_fd, "pong", 4, 0) == -1) {
                        perror("send");
                        goto server_conn_cleanup;
                    }
                } else if (strcmp(cmd, "exit") == 0) {
                    close(conn_sock_fd);
                    close(listen_sock_fd);
                    printf("Exited.\n");
                    exit(1);
                } else {
                    if (send(conn_sock_fd,
                             "Invalid command!",
                             16, 0) == -1
                    ) {
                        perror("send");
                        goto server_conn_cleanup;
                    }
                }
            }

            server_conn_cleanup:
                close(conn_sock_fd);
        }
    } else if (argc > 1 && strcmp(argv[1], "client") == 0) {
        memset(&addrinfo_hints, 0, sizeof(addrinfo_hints));
        addrinfo_hints.ai_family   = AF_UNSPEC;
        addrinfo_hints.ai_socktype = SOCK_STREAM;

        struct addrinfo* addrinfo_result;
        if (getaddrinfo(
            "127.0.0.1", "8080",
            &addrinfo_hints, &addrinfo_result) != 0
        ) {
            perror("getaddrinfo");
            exit(1);
        }

        int conn_sock_fd = -1;
        bool connected_socket = false;
        for (struct addrinfo* p = addrinfo_result;
             p != NULL;
             p = p->ai_next
        ) {
            conn_sock_fd = socket(p->ai_family,
                                  p->ai_socktype,
                                  p->ai_protocol);
            if (conn_sock_fd == -1)
                continue;

            if (connect(conn_sock_fd,
                        p->ai_addr,
                        p->ai_addrlen) == -1
            ) {
                close(conn_sock_fd);
                continue;
            }

            connected_socket = true;
            break;
        }
        freeaddrinfo(addrinfo_result);
        if (!connected_socket) {
            perror("Failed to bind IPv4 and IPv6");
            exit(1);
        }

        while (true) {
            char cmd[1024];
            int n = read(STDIN_FILENO, cmd, 1024);
            if (n == -1) {
                perror("read");
                goto client_cleanup;
            }
            cmd[n - 1] = '\0';

            if (send(conn_sock_fd, cmd, strlen(cmd), 0) == -1) {
                perror("send");
                goto client_cleanup;
            }

            char response[1024];
            n = recv(conn_sock_fd, response, 1024, 0);
            response[n] = '\0';
            if (n == 0) {
                break;
            } else if (n != -1) {
                printf("%s\n", response);
            } else {
                perror("recv");
                goto client_cleanup;
            }
        }

        client_cleanup:
            close(conn_sock_fd);
    } else {
        fprintf(stderr,
                "First argument must be 'server' or 'client'");
        exit(1);
    }

    exit(0);
}

