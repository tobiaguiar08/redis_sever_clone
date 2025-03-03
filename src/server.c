#include "server.h"
#include "cmd_hdlr.h"
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

#define MAX_ARGS 3

static int server_running = 1;

void handle_sigint(int sig) {
    (void)sig;
    server_running = 0;
}

static int parsed_hdlr_to_cmd_format(struct resp_protocol_hdlr *hdlr, int *argc, char **argv)
{
    int i;

    if (!hdlr || !argc || !argv)
        return -1;

    if (!hdlr->buf_arr || !hdlr->arr_size)
        return -1;

    *argc = hdlr->arr_size;

    for (i = 0; i < *argc; i++) {
        if (!hdlr->buf_arr[i].data)
            return -1;

        argv[i] = hdlr->buf_arr[i].data;
    }

    return 0;
}

int start_server(unsigned int port)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;
    errno = 0;
    char buffer[64];
    char response[64];
    size_t bytes;
    int tmp_argc;
    char *tmp_argv[MAX_ARGS];
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
            if (!parsed_hdlr_to_cmd_format(resp_hdlr, &tmp_argc, tmp_argv)) {
                struct cmd_hdlr *cmd;
                if (!exec_cmd((void **)&cmd, tmp_argc, (const char **)tmp_argv)) {
                    if (strcmp(cmd->name, "ECHO")) {
                        bytes = snprintf(response, 64, "+%s\r\n", cmd->response_str);
                    } else if (strcmp(cmd->name, "PING")) {
                        bytes = snprintf(response, 64, "+%s\r\n", cmd->response_str);
                    } else {
                        bytes = snprintf(response, 64, "-ERR in parsed frame format\r\n");
                    }
                } else {
                    bytes = snprintf(response, 64, "-ERR %s\r\n", cmd->response_str);
                }
            } else {
                bytes = snprintf(response, 64, "-ERR in parsed frame format\r\n");
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
