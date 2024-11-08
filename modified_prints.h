#ifndef ERR_PRINT_HEADER
#define ERR_PRINT_HEADER

void err_print(struct shell_context *shell_ctx,
                      shell_errs_t   err_code);

void sh_print(struct shell_context *shell_ctx,
                     const char    *msg);

struct err_msg
{
    shell_errs_t err_code;

    const char *msg;
};

static const struct err_msg ERR_MSG_ARR[] =
{
    #define SHELL_ERROR_CODE(err_code, msg, ...) {err_code, msg},
    #include "shell_errs_code.gen"
    #undef SHELL_ERROR_CODE
};


#endif
