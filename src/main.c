#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "input.h"

int main(int argc, char **argv)
{
    Args args;
    AddressList addresses;
    char err[256];

    if (args_parse(argc, argv, &args, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        args_print_usage(stderr, argc > 0 ? argv[0] : "vm");
        return EXIT_FAILURE;
    }
    if (addresses_load(args.address_path, &addresses, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        return EXIT_FAILURE;
    }
    printf("%zu enderecos lidos de %s\n", addresses.count, args.address_path);
    addresses_free(&addresses);
    return EXIT_SUCCESS;
}
