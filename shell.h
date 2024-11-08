#ifndef SHELL_HEADER
#define SHELL_HEADER

#include <readline/history.h>
#include <readline/readline.h>
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

#define _GNU_SOURCE

static const char *prompt_line = "+-->";

static const char *cmd_delimiters = " \n\r\t\b";

static const char *pipe_token = "|";

typedef enum
{
    SHELL_CD,
    NOT_SHELL_FUNC,
} shell_func_code_t;

typedef struct
{
    const char *name;

    shell_func_code_t code;
} shell_func_t;

static shell_func_t SHELL_FUNCS[] = {{"cd", SHELL_CD}};

static const size_t SHELL_FUNCS_COUNT = sizeof(SHELL_FUNCS) / sizeof(*SHELL_FUNCS);

typedef enum
{
    #define SHELL_ERROR_CODE(err_code, ...) err_code,
    #include "shell_errs_code.gen"
    #undef SHELL_ERROR_CODE
} shell_errs_t;

struct tokens
{
    char *buf;

    size_t buf_len;

    char ** token_array;

    size_t tokens_count;
};

struct command_list_node
{
    char  *cmd_name;
    char **args;

    int in_fd;
    int out_fd;

    struct command_list_node *next_cmd;
    struct command_list_node *prev_cmd;
};

#define MAX_CWD_BUF_SIZE 256

struct shell_context
{
    char  cwd[MAX_CWD_BUF_SIZE];

    const char *user_name;
    const char *prog_name;

    struct tokens tokens;
};

void print_prompt      (struct shell_context *sh_ctx);
void print_hail_message(struct shell_context *sh_ctx);

shell_errs_t call_user_interface(struct shell_context *sh_ctx);
shell_errs_t exec_command       (struct shell_context *sh_ctx);

shell_errs_t read_command            (struct tokens *tokens);
shell_errs_t split_buffer_into_tokens(struct tokens *tokens);
shell_errs_t tokens_destroy_buf      (struct tokens *tokens);
shell_errs_t tokens_destroy_token_arr(struct tokens *tokens);

void print_tokens(struct tokens *tokens);

shell_func_code_t get_shell_func_code(const char *func_name);

#endif
