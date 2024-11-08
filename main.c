#include <stdbool.h>

#include "shell_context.h"
#include "shell.h"
#include "modified_prints.h"

// TODO: split on files and dirs

int main(int argc, char **argv)
{
    struct shell_context sh_ctx;

    shell_context_init(&sh_ctx, argc, argv);

    print_hail_message(&sh_ctx);

    if (call_user_interface(&sh_ctx) != SHELL_SUCCESS)
    {
        return errno;
    }

    return 0;
}
