#ifndef _CMD_HDLR_H
#define _CMD_HDLR_H

#define MAX_CMDS 2

#define MAX_NAME_LEN 10

typedef int (*handler_cb)(void *ctx, int argc, const char **argv);

#define MAX_RESPONSE_SIZE 1024

struct cmd_hdlr {
  const char name[MAX_NAME_LEN];
  const char *help_str;
  handler_cb cmd_cb;
  char response_str[MAX_RESPONSE_SIZE];
  void *ctx;
};

extern struct cmd_hdlr *cmds_list[MAX_CMDS];

int find_cmd(const char *cmd_name);
int exec_cmd(void **ctx, int argc, const char **argv);

int handle_echo_command(void *ctx, int argc, const char **argv);
int handle_ping_command(void *ctx, int argc, const char **argv);

#endif // !_CMD_HDLR_H
