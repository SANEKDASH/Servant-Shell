#include "shell.h"
#include "modified_prints.h"
#include "color_print.h"

void err_print(struct shell_context *shell_ctx, shell_errs_t err_code)
{
    printf("%s: %s. Reason: ", shell_ctx->prog_name, ERR_MSG_ARR[err_code].msg);

    color_printf(kRed, "%s.\n", strerror(errno));
}

void sh_print(struct shell_context *shell_ctx,
                     const char    *msg)
{
    printf("%s: Your majesty %s. %s.\n", shell_ctx->prog_name,
                                         shell_ctx->user_name,
                                         msg);
}
