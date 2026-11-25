#include "args.h"

#define EXPECTED_ARGS 3

int args_parse(int argc, char **argv, Args *args, char *err, size_t err_size)
{
    int received = argc > 0 ? argc - 1 : 0;

    if (received != EXPECTED_ARGS) {
        snprintf(err, err_size, "numero de argumentos invalido (esperado %d, recebido %d)",
                 EXPECTED_ARGS, received);
        return -1;
    }
    if (argv[1][0] == '\0') {
        snprintf(err, err_size, "o caminho do arquivo de enderecos esta vazio");
        return -1;
    }
    if (policy_parse(argv[2], &args->page_policy) != 0) {
        snprintf(err, err_size, "algoritmo de substituicao de pagina invalido '%s' (use fifo ou lru)",
                 argv[2]);
        return -1;
    }
    if (policy_parse(argv[3], &args->tlb_policy) != 0) {
        snprintf(err, err_size, "algoritmo de substituicao da TLB invalido '%s' (use fifo ou lru)",
                 argv[3]);
        return -1;
    }
    args->address_path = argv[1];
    return 0;
}

void args_print_usage(FILE *out, const char *program)
{
    fprintf(out, "uso: %s <arquivo_de_enderecos> <fifo|lru> <fifo|lru>\n", program);
    fprintf(out, "  arquivo_de_enderecos  enderecos logicos, um inteiro por linha\n");
    fprintf(out, "  fifo|lru              substituicao de pagina\n");
    fprintf(out, "  fifo|lru              substituicao da TLB\n");
}
