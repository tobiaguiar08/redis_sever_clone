#include "server.h"
#include "protocol.h"

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
    char response[16];
    size_t bytes;
    struct resp_protocol_hdlr *resp_hdlr;

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

    resp_hdlr = create_protocol_handler();
    if (!resp_hdlr) {
        printf("error in protocol handler creation\n");
        goto out;
    }

    while (server_running) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected\n");

        while (server_running) {
            ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                printf("Client disconnected\n");
                break;
            }

            buffer[bytes_read] = '\0';

            parse_frame(buffer, resp_hdlr);
            if (resp_hdlr->type == '+' || resp_hdlr->type == '-') {
                bytes = snprintf(response, sizeof(response), "%s", (const char *)resp_hdlr->data.p_data);
            } else if (resp_hdlr->type == ':') {
                bytes = snprintf(response, sizeof(response), "%d", *(int *)resp_hdlr->data.p_data);
            }
            send(new_socket, response, bytes, 0);
            destroy_protocol_handler_data(resp_hdlr);
        }

        close(new_socket);
    }

    destroy_protocol_handler(resp_hdlr);
out:
    close(server_fd);
    return 0;
}
