#ifndef SHELL_CONTEXT_HEADER
#define SHELL_CONTEXT_HEADER

#include "shell.h"

shell_errs_t shell_context_init   (struct shell_context *sh_ctx, int argc, char **argv);
shell_errs_t shell_context_destroy(struct shell_context *sh_ctx);

#endif
