#include "cmd_hdlr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "protocol.h"

int handle_echo_command(void *ctx, int argc, const char **argv)
{
    struct cmd_hdlr *cmd = (struct cmd_hdlr *)ctx;
    struct resp_protocol_hdlr *resp = &cmd->resp;
    int rc = 0, written;

    memset(cmd->response_str, 0, MAX_RESPONSE_SIZE);
    if (!resp->buf_arr) {
        resp->buf_arr = calloc(1, sizeof(struct str_type_buffer));
        resp->arr_size = 1;
    }

    if (argc != 2) {
        resp->type = '-';
        written = snprintf(cmd->response_str, MAX_RESPONSE_SIZE, "argument number incorrect. usage :\n\t[%s]", cmd->help_str);

        rc = -1;
    } else {
        resp->type = '$';
        written = snprintf(cmd->response_str, MAX_RESPONSE_SIZE, "%s", argv[1]);
    }

    if (written >= MAX_RESPONSE_SIZE) {
        cmd->response_str[MAX_RESPONSE_SIZE - 1] = '\0';
        resp->buf_arr[0].length = MAX_RESPONSE_SIZE - 1;
    } else {
        cmd->response_str[written] = '\0';
        resp->buf_arr[0].length = written;
    }

    resp->buf_arr[0].data = cmd->response_str;

    return rc;
}

static struct cmd_hdlr echo_cmd = {
    .name = "ECHO",
    .help_str = "ECHO <string_to_echo>",
    .cmd_cb = handle_echo_command,
    .response_str = {0},
    .ctx = &echo_cmd
};

int handle_ping_command(void *ctx, int argc, const char **argv)
{
    struct cmd_hdlr *cmd = (struct cmd_hdlr *)ctx;
    struct resp_protocol_hdlr *resp = &cmd->resp;
    int rc = 0, written;

    memset(cmd->response_str, 0, sizeof(cmd->response_str));
    if (!resp->buf_arr) {
        resp->buf_arr = calloc(1, sizeof(struct str_type_buffer));
        resp->arr_size = 1;
    }

    if (argc != 1) {
        resp->type = '-';
        written = snprintf(cmd->response_str, MAX_RESPONSE_SIZE, "argument number incorrect. usage :\n\t[%s]", cmd->help_str);

        rc = -1;
    } else {
        resp->type = '+';
        written = snprintf(cmd->response_str, MAX_RESPONSE_SIZE, "PONG");
    }

    if (written >= MAX_RESPONSE_SIZE) {
        cmd->response_str[MAX_RESPONSE_SIZE - 1] = '\0';
        resp->buf_arr[0].length = MAX_RESPONSE_SIZE - 1;
    } else {
        cmd->response_str[written] = '\0';
        resp->buf_arr[0].length = written;
    }

    resp->buf_arr[0].data = cmd->response_str;

    return rc;
}

static struct cmd_hdlr ping_cmd = {
    .name = "PING",
    .help_str = "PING",
    .cmd_cb = handle_ping_command,
    .response_str = {0},
    .ctx = &ping_cmd
};

struct cmd_hdlr *cmds_list[MAX_CMDS] = {
    &echo_cmd,
    &ping_cmd
};

static struct cmd_hdlr unknown_cmd = {
    .name = "UNKNOWN",
    .help_str = "Unknown command",
    .response_str = {0},
    .ctx = &unknown_cmd
};

int find_cmd(const char *input_cmd_name)
{
    unsigned int cmd_size = sizeof(cmds_list)/sizeof(cmds_list[0]);
    int i;

    for (i = 0; i < cmd_size; i++) {
        struct cmd_hdlr *cmd = cmds_list[i];
        if (!strncmp(input_cmd_name, cmd->name, strlen(input_cmd_name)))
            return i;
    }

    return -1;
}

int exec_cmd(void **ctx, int argc, const char **argv)
{
    struct cmd_hdlr *cmd;
    struct resp_protocol_hdlr *resp;
    int cmd_idx, bytes;

    cmd_idx = find_cmd(argv[0]);
    if (cmd_idx < 0) {
        cmd = &unknown_cmd;
        resp = &cmd->resp;
        if (!resp->buf_arr) {
            resp->buf_arr = calloc(1, sizeof(struct str_type_buffer)); 
        }

        resp->type = '-';
        bytes = snprintf(cmd->response_str, MAX_RESPONSE_SIZE, "%s: %s", unknown_cmd.help_str, argv[0]);
        cmd->response_str[bytes] = '\0';
        resp->buf_arr[0].data = cmd->response_str;
        *ctx = cmd;
        return -1;
    }

    *ctx = cmds_list[cmd_idx];
    cmd = *(struct cmd_hdlr **)ctx;
    return cmd->cmd_cb(cmd, argc, argv);
}
