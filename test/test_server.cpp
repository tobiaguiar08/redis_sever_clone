#include <CppUTest/UtestMacros.h>
#include "CppUTest/TestHarness.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <cstring>
#include <signal.h>

extern "C"
{
#include "server.h"
}

TEST_GROUP(TestGroupServer) {
    int client_socket;
    int server_pid;

    void setup() {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        CHECK(client_socket >= 0);
    }

    void teardown() {
        if (client_socket >= 0) {
            close(client_socket);
        }
        if (server_pid == 0) {
            kill(server_pid, SIGINT);
            waitpid(server_pid, nullptr, 0);
        }
    }
};

TEST(TestGroupServer, TestServerHandlesMultipleFrames) {
    server_pid = fork();

    if (server_pid == 0) {
        start_server(12345);
        exit(0);
    }

    sleep(1);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    CHECK(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0);

    const char *message1 = "*1\r\n$4\r\nPING\r\n";
    send(client_socket, message1, strlen(message1), 0);

    char buffer1[1024] = {0};
    ssize_t bytes_received1 = recv(client_socket, buffer1, sizeof(buffer1), 0);
    CHECK(bytes_received1 > 0);
    STRNCMP_EQUAL("+PONG\r\n", buffer1, strlen("+PONG\r\n"));

    const char *message2 = "*2\r\n$4\r\nECHO\r\n$5\r\nHELLO\r\n";
    send(client_socket, message2, strlen(message2), 0);

    char buffer2[1024] = {0};
    ssize_t bytes_received2 = recv(client_socket, buffer2, sizeof(buffer2), 0);
    CHECK(bytes_received2 > 0);
    STRNCMP_EQUAL("+HELLO\r\n", buffer2, strlen("+HELLO\r\n"));
}
