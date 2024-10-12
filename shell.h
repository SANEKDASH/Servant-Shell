#ifndef SHELL_HEADER
#define SHELL_HEADER

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

static const char *prompt_line = "+--> ";

static const char *cmd_delimiters = " \n\r\t\b";

static const char *pipe_token = "|";

typedef enum
{
    SHELL_SUCCESS,
    SHELL_ERR,
    PARSE_ERR,
    EMPTY_LINE,
    SHELL_EOF,
    PIPE_ERR,
} shell_errs_t;

struct tokens_t
{
    char *buf;
    char ** token_array;

    int read_size;

    size_t buf_size;
    size_t tokens_count;
};

struct command_list_node_t
{
    char  *cmd_name;
    char **args;

    int in_fd;
    int out_fd;

    struct command_list_node_t *next_cmd;
    struct command_list_node_t *prev_cmd;
};

void print_prompt      (struct passwd *cur_user_passwd);
void print_hail_message(struct passwd *cur_user_passwd, const char *prog_name);

shell_errs_t exec_command(struct tokens_t *tokens);

shell_errs_t parse_command(struct tokens_t *tokens);

shell_errs_t read_command(struct tokens_t *tokens);

shell_errs_t split_buffer_into_tokens(struct tokens_t *tokens);

shell_errs_t tokens_destroy_buf      (struct tokens_t *tokens);
shell_errs_t tokens_destroy_token_arr(struct tokens_t *tokens);

void tokens_init(struct tokens_t *tokens);

void print_tokens(struct tokens_t *tokens);

#endif
