#include "shell_context.h"

shell_errs_t shell_context_init(struct shell_context *sh_ctx, int argc, char **argv)
{
    uid_t cur_uid = getuid();

    struct passwd *cur_user_passwd = getpwuid(cur_uid);

    if (cur_user_passwd == NULL)
    {
        printf("%s: I failed to get information about you, my Lord.\n", argv[0]);

        return 0;
    }

    sh_ctx->user_name = cur_user_passwd->pw_name;
    sh_ctx->prog_name = argv[0];

    sh_ctx->tokens.token_array = NULL;
    sh_ctx->tokens.buf         = NULL;

    if (getcwd(sh_ctx->cwd, MAX_CWD_BUF_SIZE) == NULL)
    {
        return SHELL_ERR;
    }

    return SHELL_SUCCESS;
}

shell_errs_t shell_context_destroy(struct shell_context *sh_ctx)
{
    return SHELL_SUCCESS;
}
