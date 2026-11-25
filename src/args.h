#ifndef ARGS_H
#define ARGS_H

#include <stddef.h>
#include <stdio.h>

#include "policy.h"

typedef struct {
    const char *address_path;
    Policy page_policy;
    Policy tlb_policy;
} Args;

/**
 * Valida a linha de comando: <arquivo_de_enderecos> <fifo|lru> <fifo|lru>.
 * Retorna 0 e preenche args, ou -1 e escreve a causa em err.
 */
int args_parse(int argc, char **argv, Args *args, char *err, size_t err_size);

/** Escreve em out a forma de uso do programa. */
void args_print_usage(FILE *out, const char *program);

#endif
