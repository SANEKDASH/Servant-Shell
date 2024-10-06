#include "shell.h"

static int split_buffer_into_strings(char *cmd_buf);

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
    printf("+--[%s]\n", cur_user_passwd->pw_name);

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

shell_errs_t exec_command(struct tokens_t *tokens)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        if (execvp(tokens->token_array[0], tokens->token_array) < 0)
        {
            perror("failed to exec");

            _exit(errno);
        }
    }

    if (pid < 0)
    {
        perror("failed to fork\n");
    }

    int wait_status = 0;

    if (wait(&wait_status) < 0)
    {
        perror("wait failure");

        return SHELL_ERR;
    }

    return SHELL_SUCCESS;
}

//=============================================================================

void tokens_init(struct tokens_t *tokens)
{
    tokens->buf_size = MAX_CMD_LEN;

    tokens->buf = (char *) malloc(sizeof(char) * MAX_CMD_LEN);
}

//=============================================================================
