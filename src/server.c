#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>

static int server_running = 1;

void handle_sigint(int sig) {
    (void)sig;
    server_running = 0;
}


int start_server(unsigned int port)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;
    errno = 0;
    char buffer[64];

    signal(SIGINT, handle_sigint);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0 && errno != 0) {
        printf("err %s\n", strerror(errno));
        return -1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Bind failed");
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        printf("Listen failed");
        return -1;
    }

    printf("\nServer listening on port %d...\n", port);

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        return -1;
    }

    while (server_running) {
        char buffer[1024] = {0};
        ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            if (strcmp(buffer, "PING") == 0) {
                send(new_socket, "PONG", 4, 0);
            } else if (strncmp(buffer, "ECHO ", 5) == 0) {
                send(new_socket, buffer + 5, strlen(buffer) - 5, 0);
            } else {
                send(new_socket, "UNKNOWN", 7, 0);
            }
        }
    }

    close(new_socket);

    return server_fd;
}
