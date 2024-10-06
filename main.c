#include <stdbool.h>

#include "shell.h"

// TODO: shell context (user, tokens, etc...), pipe

int main(int argc, char *const argv[])
{
    uid_t cur_uid = getuid();

    struct passwd *cur_user_passwd = getpwuid(cur_uid);

    if (cur_user_passwd == NULL)
    {
        printf("%s: I failed to get information about you, my Lord.\n", argv[0]);

        return 0;
    }

    print_hail_message(cur_user_passwd, argv[0]);

    struct tokens_t tokens;

    tokens_init(&tokens);

    while (true)
    {
        print_prompt(cur_user_passwd);

        shell_errs_t read_result = read_command(&tokens);

        if (read_result != SHELL_SUCCESS)
        {
            free(tokens.buf);

            return errno;
        }

        shell_errs_t parse_result = split_buffer_into_tokens(&tokens);

        switch (parse_result)
        {
            case EMPTY_LINE:
            {
                break;
            }

            case SHELL_SUCCESS:
            {
                exec_command(&tokens);

                break;
            }

            case SHELL_EOF:
            {
                free(tokens.buf);

                printf("%s: Goodbye, my Lord...\n", argv[0]);

                return 0;
            }

            default:
            {
                printf("%s: My Lord, i don't know such type of orders.\n", argv[0]);

                break;
            }
        }

        free(tokens.token_array);
    }

    return 0;
}
