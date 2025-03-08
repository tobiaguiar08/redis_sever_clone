#include <CppUTest/UtestMacros.h>
#include "CppUTest/TestHarness.h"
#include "protocol.h"

extern "C" {
#include <string.h>
#include <stdio.h>
#include "cmd_hdlr.h"
}

TEST_GROUP(CommandHdlrs) {
  void setup()
  {
  }

  void teardown()
  {
  }
};

TEST(CommandHdlrs, EchoCommandIncorrectArgc)
{
    const char *test_argv[] = {"ECHO"};
    int res, test_argc, written;

    char expected_str[MAX_RESPONSE_SIZE];
    struct resp_protocol_hdlr response_resp;
    memset(&response_resp, 0, sizeof(response_resp));

    struct cmd_hdlr test_echo_cmd = {
        .name = "ECHO",
        .help_str = "ECHO <string_to_echo>",
        .cmd_cb = handle_echo_command,
        .response_str = {0},
        .resp = response_resp,
        .ctx = &test_echo_cmd
    };

    written = snprintf(expected_str, MAX_RESPONSE_SIZE, "argument number incorrect. usage :\n\t[%s]", test_echo_cmd.help_str);
    if (written >= MAX_RESPONSE_SIZE)
        expected_str[MAX_RESPONSE_SIZE - 1] = '\0';
    else
        expected_str[written] = '\0';

    test_argc = 1;

    res = test_echo_cmd.cmd_cb(test_echo_cmd.ctx, test_argc, test_argv);
    CHECK(res == -1);
    STRNCMP_EQUAL(expected_str, test_echo_cmd.response_str, sizeof(test_echo_cmd.response_str));
}

TEST(CommandHdlrs, EchoCommand)
{
    const char *test_argv[] = {"ECHO", "HELLO"};
    int res, test_argc, written;

    char expected_str[MAX_RESPONSE_SIZE];
    struct resp_protocol_hdlr response_resp;
    memset(&response_resp, 0, sizeof(response_resp));

    struct cmd_hdlr test_echo_cmd = {
        .name = "ECHO",
        .help_str = "ECHO <string_to_echo>",
        .cmd_cb = handle_echo_command,
        .response_str = {0},
        .resp = response_resp,
        .ctx = &test_echo_cmd
    };

    written = snprintf(expected_str, MAX_RESPONSE_SIZE, "%s", test_argv[1]);
    if (written >= MAX_RESPONSE_SIZE)
        expected_str[MAX_RESPONSE_SIZE - 1] = '\0';
    else
        expected_str[written] = '\0';

    test_argc = 2;

    res = test_echo_cmd.cmd_cb(test_echo_cmd.ctx, test_argc, test_argv);
    CHECK(res == 0);
    CHECK(test_echo_cmd.resp.buf_arr != NULL);
    STRNCMP_EQUAL(expected_str, test_echo_cmd.response_str, MAX_RESPONSE_SIZE);
    STRNCMP_EQUAL(expected_str, (const char *)test_echo_cmd.resp.buf_arr[0].data, test_echo_cmd.resp.buf_arr[0].length);
}

TEST(CommandHdlrs, PingCommand)
{
    const char *test_argv[] = {"PING"};
    int res, test_argc, written;

    char expected_str[MAX_RESPONSE_SIZE];
    struct resp_protocol_hdlr response_resp;
    memset(&response_resp, 0, sizeof(response_resp));

    struct cmd_hdlr test_ping_cmd = {
        .name = "PING",
        .help_str = "PING",
        .cmd_cb = handle_ping_command,
        .response_str = {0},
        .resp = response_resp,
        .ctx = &test_ping_cmd
    };

    written = snprintf(expected_str, MAX_RESPONSE_SIZE, "PONG");
    if (written >= MAX_RESPONSE_SIZE)
        expected_str[MAX_RESPONSE_SIZE - 1] = '\0';
    else
        expected_str[written] = '\0';

    test_argc = 1;

    res = test_ping_cmd.cmd_cb(test_ping_cmd.ctx, test_argc, test_argv);
    CHECK(res == 0);
    CHECK(test_ping_cmd.resp.buf_arr != NULL);
    STRNCMP_EQUAL(expected_str, test_ping_cmd.response_str, MAX_RESPONSE_SIZE);
    STRNCMP_EQUAL(expected_str, (const char *)test_ping_cmd.resp.buf_arr[0].data, test_ping_cmd.resp.buf_arr[0].length);
}

TEST(CommandHdlrs, TestFindCommand)
{
    const char *test_argv0[] = {"ECHO", "HI"};
    const char *test_argv1[] = {"PING"};
    const char *test_argv2[] = {"ERROR"};
    int res;

    res = find_cmd(test_argv0[0]);
    CHECK(res >= 0);

    res = find_cmd(test_argv1[0]);
    CHECK(res >= 0);

    res = find_cmd(test_argv2[0]);
    CHECK(res < 0);
}

TEST(CommandHdlrs, TestExecCommand)
{
    const char *test_argv0[] = {"ECHO", "HI"};
    int test_argc0 = 2;
    const char *expected_out0 = test_argv0[1];
    const char *test_argv1[] = {"PING"};
    int test_argc1 = 1;
    char expected_out1[5];
    const char *test_argv2[] = {"ERROR"};
    int test_argc2 = 1;
    char expected_out2[50];
    struct cmd_hdlr *test_cmd;
    int res;

    snprintf(expected_out1, 5, "PONG");
    snprintf(expected_out2, 50, "Unknown command: %s", test_argv2[0]);

    res = exec_cmd((void **)&test_cmd, test_argc0, test_argv0);
    CHECK(res >= 0);
    STRNCMP_EQUAL(expected_out0, test_cmd->response_str, strlen(expected_out0));

    res = exec_cmd((void **)&test_cmd, test_argc1, test_argv1);
    CHECK(res >= 0);
    STRNCMP_EQUAL(expected_out1, test_cmd->response_str, strlen(expected_out1));

    res = exec_cmd((void **)&test_cmd, test_argc2, test_argv2);
    CHECK(res < 0);
    STRNCMP_EQUAL(expected_out2, test_cmd->response_str, strlen(expected_out2));
}
