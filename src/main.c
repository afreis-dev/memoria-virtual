#include <stdio.h>
#include <stdlib.h>

#include "address.h"
#include "args.h"
#include "input.h"

int main(int argc, char **argv)
{
    Args args;
    AddressList addresses;
    char err[256];
    size_t i;

    if (args_parse(argc, argv, &args, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        args_print_usage(stderr, argc > 0 ? argv[0] : "vm");
        return EXIT_FAILURE;
    }
    if (addresses_load(args.address_path, &addresses, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        return EXIT_FAILURE;
    }
    for (i = 0; i < addresses.count; i++) {
        LogicalAddress split = address_split(addresses.values[i]);

        printf("Virtual address: %u Page: %u Offset: %u\n", split.address, split.page, split.offset);
    }
    addresses_free(&addresses);
    return EXIT_SUCCESS;
}
