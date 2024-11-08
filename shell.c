#include "shell.h"
#include "color_print.h"
#include "modified_prints.h"

static int split_buffer_into_strings(char *cmd_buf);

static struct command_list_node *get_command_list(struct tokens *tokens);

static shell_errs_t command_list_destroy(struct command_list_node *cmd_list);

static shell_errs_t exec_shell_func(struct shell_context     *sh_ctx,
                                    shell_func_code_t         func_code,
                                    struct command_list_node *cmd_node);

#define  SH_PRINT(msg)       sh_print (sh_ctx, msg)
#define ERR_PRINT(err_code) err_print(sh_ctx, err_code)

#define DO_WITH_ERR_CHECK(func_to_call, what_to_do_in_case_of_error) \
    {                                                                \
        shell_errs_t func_err_code = func_to_call;                   \
                                                                     \
        if (func_err_code != SHELL_SUCCESS)                          \
        {                                                            \
            ERR_PRINT(func_err_code);                                \
        }                                                            \
                                                                     \
        what_to_do_in_case_of_error                                  \
    }

//=============================================================================

void print_tokens(struct tokens *tokens)
{
    for (int i = 0; i < tokens->tokens_count; i++)
    {
        printf("%s\n", tokens->token_array[i]);
    }
}

//=============================================================================

void print_hail_message(struct shell_context *sh_ctx)
{
    printf("%s: Hello, my lord, %s. Today i'm at your service.\n", sh_ctx->prog_name,
                                                                   sh_ctx->user_name);
}

//=============================================================================

void print_prompt(struct shell_context *sh_ctx)
{
    printf("+--[\033[0;31m%s\033[0m]-[%s]\n|\n", sh_ctx->user_name, sh_ctx->cwd);
}

//=============================================================================

shell_errs_t call_user_interface(struct shell_context *sh_ctx)
{
    while (true)
    {
        print_prompt(sh_ctx);

        shell_errs_t read_result = read_command(&(sh_ctx->tokens));

        if (errno != 0)
        {
            ERR_PRINT(SHELL_ERR);

            return SHELL_ERR;
        }

        shell_errs_t parse_result = split_buffer_into_tokens(&(sh_ctx->tokens));

        switch (parse_result)
        {
            case EMPTY_LINE:
            {
                break;
            }

            case SHELL_SUCCESS:
            {
                DO_WITH_ERR_CHECK(exec_command(sh_ctx), /*do nothing*/);

                break;
            }

            case SHELL_EOF:
            {
                free(sh_ctx->tokens.buf);

                SH_PRINT("Nighty night..");

                return 0;
            }

            default:
            {
                SH_PRINT("I don't know such type of orders");

                break;
            }
        }

        free(sh_ctx->tokens.token_array);
        sh_ctx->tokens.token_array = NULL;
    }

    return SHELL_SUCCESS;
}

//=============================================================================

static const size_t MAX_CMD_LEN = 256;

shell_errs_t split_buffer_into_tokens(struct tokens *tokens)
{
    if (tokens->buf == NULL)
    {
        return SHELL_EOF;
    }
    if (tokens->buf[0] == '\0')
    {
        return EMPTY_LINE;
    }

    add_history(tokens->buf);

    tokens->tokens_count = split_buffer_into_strings(tokens->buf);

    tokens->token_array = (char **) malloc(sizeof(char *) * tokens->tokens_count + 1);

    char *read_ptr  = tokens->buf;

    while (*read_ptr == '\0')
    {
        ++read_ptr;
    }

    for (int i = 0; i < tokens->tokens_count; i++)
    {
        tokens->token_array[i] = read_ptr;

        while (*read_ptr != '\0')
        {
            ++read_ptr;
        }

        while (*read_ptr == '\0')
        {
            ++read_ptr;
        }
    }

    tokens->token_array[tokens->tokens_count] = NULL;

    return SHELL_SUCCESS;
}

//=============================================================================

static int split_buffer_into_strings(char *cmd_buf)
{
    char *save_ptr;

    int tokens_count = 0;

    char *str;

    for (str = cmd_buf; ; tokens_count++, str = NULL)
    {
        char *token = strtok_r(str, cmd_delimiters, &save_ptr);

        if (token == NULL)
        {
            return tokens_count;
        }
    }

    return tokens_count;
}

//=============================================================================

shell_errs_t read_command(struct tokens *tokens)
{
    tokens->buf = readline("+--> ");

    if (tokens->buf == NULL)
    {
        return SHELL_ERR;
    }

    return SHELL_SUCCESS;
}

//=============================================================================

static void skip_command(struct tokens *tokens, char ***iter)
{
    while (*iter < tokens->token_array + tokens->tokens_count)
    {
        if ((**iter) == NULL)
        {
            break;
        }

        if (strcmp(**iter, pipe_token) == 0)
        {
            break;
        }

        ++(*iter);
    }
}

//=============================================================================

static bool is_next_token_enum(struct tokens *tokens, char **cur_token)
{
    if (cur_token < tokens->token_array + tokens->tokens_count)
    {
        if (*cur_token != NULL)
        {
            if (strcmp(*cur_token, pipe_token) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

//=============================================================================

static shell_errs_t command_list_destroy(struct command_list_node *cmd_list)
{
    struct command_list_node *cur_cmd_node  = cmd_list;
    struct command_list_node *prev_cmd_node = NULL;

    while(cur_cmd_node != NULL)
    {
        prev_cmd_node = cur_cmd_node;
         cur_cmd_node = cur_cmd_node->next_cmd;

        free(prev_cmd_node);
    }

    return SHELL_SUCCESS;
}

//=============================================================================

static struct command_list_node *command_list_node_create(char **args)
{
    struct command_list_node *new_cmd_node = (struct command_list_node *) malloc(sizeof(struct command_list_node));

    if (new_cmd_node == NULL)
    {
        perror("failed to allocate memory");

        return NULL;
    }

    new_cmd_node->args     =  args;
    new_cmd_node->cmd_name = *args;

    new_cmd_node->next_cmd = NULL;
    new_cmd_node->prev_cmd = NULL;

    new_cmd_node->in_fd  = STDIN_FILENO;
    new_cmd_node->out_fd = STDOUT_FILENO;

    return new_cmd_node;
}

//=============================================================================

static struct command_list_node *get_command_list(struct tokens *tokens)
{
    char **read_token  = tokens->token_array;

    struct command_list_node *cmd_node = NULL;
    struct command_list_node *cur_cmd_node = cmd_node;

    if (read_token != NULL)
    {
        if (strcmp(*read_token, pipe_token) == 0)
        {
            command_list_destroy(cmd_node);

            return NULL;
        }

        cmd_node = command_list_node_create(read_token);

        cur_cmd_node = cmd_node;

        skip_command(tokens, &read_token);
    }

    while (*read_token != NULL)
    {
        if (strcmp(*read_token, pipe_token) == 0)
        {
            *(read_token++) = NULL;
        }

        if (*read_token == NULL)
        {
            command_list_destroy(cmd_node);

            return NULL;
        }

        if (strcmp(*read_token, pipe_token) == 0)
        {
            command_list_destroy(cmd_node);

            return NULL;
        }

        cur_cmd_node->next_cmd = command_list_node_create(read_token);

        cur_cmd_node->next_cmd->prev_cmd = cur_cmd_node;

        cur_cmd_node = cur_cmd_node->next_cmd;

        skip_command(tokens, &read_token);
    }

    return cmd_node;
}

//=============================================================================

static shell_errs_t create_pipe_between_procs(struct command_list_node *lhs_proc,
                                              struct command_list_node *rhs_proc)
{
    int pipe_fds[2];

    if (rhs_proc != NULL)
    {
        int pipe_status = pipe(pipe_fds);

        if (pipe_status < 0)
        {
            return PIPE_ERR;
        }

        lhs_proc->out_fd = pipe_fds[1];

        rhs_proc->in_fd  = pipe_fds[0];
    }

    return SHELL_SUCCESS;
}

//=============================================================================

shell_errs_t exec_command(struct shell_context *sh_ctx)
{
    struct command_list_node *cmd_list = get_command_list(&sh_ctx->tokens);

    if (cmd_list == NULL)
    {
        return PARSE_ERR;
    }

    struct command_list_node *cur_cmd_node = cmd_list;

    int cmd_count = 0;

    while(cur_cmd_node != NULL)
    {
        if (cur_cmd_node->next_cmd != NULL)
        {
            DO_WITH_ERR_CHECK(create_pipe_between_procs(cur_cmd_node, cur_cmd_node->next_cmd), /*do nothing*/);
        }

        shell_func_code_t func_code = get_shell_func_code(cur_cmd_node->cmd_name);

        if (func_code == SHELL_CD)
        {
            DO_WITH_ERR_CHECK(exec_shell_func(sh_ctx, func_code, cur_cmd_node), /*do nothing*/)

            break;
        }

        pid_t new_pid = fork();

        if (new_pid == 0)
        {
            if (cur_cmd_node->in_fd != STDIN_FILENO) // reader
            {
                close(STDIN_FILENO);

                dup  (cur_cmd_node->in_fd);
                close(cur_cmd_node->in_fd);
            }

            if (cur_cmd_node->out_fd != STDOUT_FILENO)
            {
                close(STDOUT_FILENO);

                dup  (cur_cmd_node->out_fd);
                close(cur_cmd_node->out_fd);
            }

            if (func_code != NOT_SHELL_FUNC)
            {
                DO_WITH_ERR_CHECK(exec_shell_func(sh_ctx, func_code, cur_cmd_node), /*do nothing*/);

                exit(0);
            }
            else
            {
                if (execvp(cur_cmd_node->cmd_name, cur_cmd_node->args) < 0)
                {
                    ERR_PRINT(FAILED_TO_EXEC);

                    exit(errno);
                }
            }
        }

        if (cur_cmd_node->in_fd != STDIN_FILENO)
        {
            close(cur_cmd_node->in_fd);
        }

        if (cur_cmd_node->out_fd != STDOUT_FILENO)
        {
            close(cur_cmd_node->out_fd);
        }

        if (new_pid < 0)
        {
            perror("failed to fork\n");

            return SHELL_ERR;
        }

        cur_cmd_node = cur_cmd_node->next_cmd;

        ++cmd_count;
    }

    int wait_status = 0;

    for (int i = 0; i < cmd_count; i++)
    {
        wait(&wait_status);
    }

    command_list_destroy(cmd_list);

    return SHELL_SUCCESS;
}

//=============================================================================

shell_errs_t tokens_destroy_buf(struct tokens *tokens)
{
    free(tokens->buf);

    tokens->buf = NULL;

    return SHELL_SUCCESS;
}
//=============================================================================

shell_errs_t tokens_destroy_token_arr(struct tokens *tokens)
{
    free(tokens->token_array);

    tokens->token_array = NULL;

    return SHELL_SUCCESS;
}

//=============================================================================

shell_func_code_t get_shell_func_code(const char *func_name)
{
    for (int i = 0; i < SHELL_FUNCS_COUNT; i++)
    {
        if (strcmp(SHELL_FUNCS[i].name, func_name) == 0)
        {
            return SHELL_FUNCS[i].code;
        }
    }

    return NOT_SHELL_FUNC;
}

//=============================================================================

static shell_errs_t exec_shell_func(struct shell_context     *sh_ctx,
                                           shell_func_code_t  func_code,
                                    struct command_list_node *cmd_node)
{
    switch (func_code)
    {
        case SHELL_CD:
        {
            if (*cmd_node->args == NULL)
            {
                return NULL_ARG;
            }

            if (chdir(cmd_node->args[1]) < 0)
            {
                ERR_PRINT(FAILED_CHANGE_DIR);
            }
            else
            {
                getcwd(sh_ctx->cwd, MAX_CWD_BUF_SIZE);
            }

            break;
        }

        default:
        {
            return UNKNOWN_FUNC_CODE;

            break;
        }
    }

    return SHELL_SUCCESS;
}

//=============================================================================

#undef SH_PRINT
#undef ERR_PRINT
