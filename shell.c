#include "shell.h"
#include "color_print.h"

static int split_buffer_into_strings(char *cmd_buf);

static struct command_list_node_t *get_command_list(struct tokens_t *tokens);

static shell_errs_t command_list_destroy(struct command_list_node_t *cmd_list);

void print_tokens(struct tokens_t *tokens)
{
    for (int i = 0; i < tokens->tokens_count; i++)
    {
        printf("%s\n", tokens->token_array[i]);
    }
}

//=============================================================================

void print_hail_message(struct passwd *cur_user_passwd, const char *prog_name)
{
    printf("%s: Hello, my lord, %s. Today i'm at your service.\n", prog_name,
                                                                   cur_user_passwd->pw_name);
}

//=============================================================================

void print_prompt(struct passwd *cur_user_passwd)
{
    // ugly
    printf("+--[\033[0;31m%s\033[0m]\n", cur_user_passwd->pw_name);

    write(1, prompt_line, strlen(prompt_line));
}

//=============================================================================

static const size_t MAX_CMD_LEN = 256;

shell_errs_t split_buffer_into_tokens(struct tokens_t *tokens)
{
    if (tokens->read_size == 0)
    {
        return SHELL_EOF;
    }

    if (tokens->read_size && tokens->buf[0] == '\0')
    {
        return EMPTY_LINE;
    }

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

shell_errs_t read_command(struct tokens_t *tokens)
{
    tokens->read_size = read(0, tokens->buf, tokens->buf_size - 1);

    if (tokens->read_size < 0)
    {
        perror("failed to read");

        return SHELL_ERR;
    }

    tokens->buf[tokens->read_size - 1] = '\0';

    return SHELL_SUCCESS;
}

//=============================================================================

static void skip_command(struct tokens_t *tokens, char ***iter)
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

static bool is_next_token_enum(struct tokens_t *tokens, char **cur_token)
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

static shell_errs_t command_list_destroy(struct command_list_node_t *cmd_list)
{
    struct command_list_node_t *cur_cmd_node  = cmd_list;
    struct command_list_node_t *prev_cmd_node = NULL;

    while(cur_cmd_node != NULL)
    {
        prev_cmd_node = cur_cmd_node;
         cur_cmd_node = cur_cmd_node->next_cmd;

        free(prev_cmd_node);
    }

    return SHELL_SUCCESS;
}

//=============================================================================

static struct command_list_node_t *command_list_node_create(char **args)
{
    struct command_list_node_t *new_cmd_node = (struct command_list_node_t *) malloc(sizeof(struct command_list_node_t));

    new_cmd_node->args     = args;
    new_cmd_node->cmd_name = *args;

    new_cmd_node->next_cmd = NULL;
    new_cmd_node->prev_cmd = NULL;

    new_cmd_node->in_fd  = STDIN_FILENO;
    new_cmd_node->out_fd = STDOUT_FILENO;

    return new_cmd_node;
}

//=============================================================================

static struct command_list_node_t *get_command_list(struct tokens_t *tokens)
{
    char **read_token  = tokens->token_array;

    struct command_list_node_t *cmd_node = NULL;
    struct command_list_node_t *cur_cmd_node = cmd_node;

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

static shell_errs_t create_pipe_between_procs(struct command_list_node_t *lhs_proc,
                                              struct command_list_node_t *rhs_proc)
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

shell_errs_t exec_command(struct tokens_t *tokens)
{
    struct command_list_node_t *cmd_list = get_command_list(tokens);

    if (cmd_list == NULL)
    {
        return PARSE_ERR;
    }

    struct command_list_node_t *cur_cmd_node = cmd_list;

    int cmd_count = 0;

    while(cur_cmd_node != NULL)
    {
        if (cur_cmd_node->next_cmd != NULL)
        {
            create_pipe_between_procs(cur_cmd_node, cur_cmd_node->next_cmd);
        }

        pid_t new_pid = fork();

        if (new_pid == 0)
        {
            if (cur_cmd_node->in_fd != STDIN_FILENO) // reader
            {
                close(STDIN_FILENO);

                dup  (cur_cmd_node->in_fd);
                close(cur_cmd_node->in_fd);

                close(cur_cmd_node->prev_cmd->out_fd); // not a writer
            }

            if (cur_cmd_node->out_fd != STDOUT_FILENO)
            {
                close(STDOUT_FILENO);

                dup  (cur_cmd_node->out_fd);
                close(cur_cmd_node->out_fd);

                close(cur_cmd_node->next_cmd->in_fd); // not a reader
            }

            if (execvp(cur_cmd_node->cmd_name, cur_cmd_node->args) < 0)
            {
                perror("failed to exec");

                exit(errno);
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

void tokens_init(struct tokens_t *tokens)
{
    tokens->buf_size = MAX_CMD_LEN;

    tokens->buf = (char *) malloc(sizeof(char) * MAX_CMD_LEN);

    tokens->read_size = 0;
}

//=============================================================================

shell_errs_t tokens_destroy_buf(struct tokens_t *tokens)
{
    free(tokens->buf);

    tokens->buf = NULL;

    return SHELL_SUCCESS;
}
//=============================================================================

shell_errs_t tokens_destroy_token_arr(struct tokens_t *tokens)
{
    free(tokens->token_array);

    tokens->token_array = NULL;

    return SHELL_SUCCESS;
}

//=============================================================================
