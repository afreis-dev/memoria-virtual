#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "args.h"
#include "backing_store.h"
#include "input.h"
#include "output.h"
#include "vmm.h"

#define BACKING_STORE_PATH "BACKING_STORE.bin"
#define OUTPUT_PATH "correct.txt"

/**
 * Traduz todos os enderecos, grava o resultado em correct.txt e o resumo no
 * terminal. Retorna 0, ou -1 com a causa em err.
 */
static int run(const AddressList *addresses, char *err, size_t err_size)
{
    BackingStore store;
    Vmm vmm;
    FILE *out;
    size_t i;
    int write_failed;
    int status = 0;

    if (backing_store_open(&store, BACKING_STORE_PATH, err, err_size) != 0) {
        return -1;
    }
    out = fopen(OUTPUT_PATH, "w");
    if (out == NULL) {
        snprintf(err, err_size, "nao foi possivel criar %s: %s", OUTPUT_PATH, strerror(errno));
        backing_store_close(&store);
        return -1;
    }

    vmm_init(&vmm, &store);
    for (i = 0; i < addresses->count && status == 0; i++) {
        LogicalAddress split = address_split(addresses->values[i]);
        Access access;

        status = vmm_access(&vmm, split, &access, err, err_size);
        if (status == 0) {
            output_write_access(out, split.address, &access);
        }
    }
    if (status == 0) {
        output_write_summary(out, &vmm);
    }
    write_failed = ferror(out) != 0;
    if (fclose(out) != 0 || write_failed) {
        if (status == 0) {
            snprintf(err, err_size, "falha ao gravar %s", OUTPUT_PATH);
        }
        status = -1;
    }
    if (status != 0) {
        remove(OUTPUT_PATH);
    } else {
        output_write_summary(stdout, &vmm);
    }
    backing_store_close(&store);
    return status;
}

int main(int argc, char **argv)
{
    Args args;
    AddressList addresses;
    char err[256];
    int status;

    if (args_parse(argc, argv, &args, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        args_print_usage(stderr, argc > 0 ? argv[0] : "vm");
        return EXIT_FAILURE;
    }
    if (addresses_load(args.address_path, &addresses, err, sizeof err) != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
        return EXIT_FAILURE;
    }
    status = run(&addresses, err, sizeof err);
    if (status != 0) {
        fprintf(stderr, "vm: erro: %s\n", err);
    }
    addresses_free(&addresses);
    return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
