#include <stdio.h>
#include <stdlib.h>

#include "args.h"

int main(int argc, char **argv)
{
    Args args;
    char err[256];

    if (args_parse(argc, argv, &args, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        args_print_usage(stderr, argc > 0 ? argv[0] : "vm");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
