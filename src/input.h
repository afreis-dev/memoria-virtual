#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t *values;
    size_t count;
} AddressList;

/**
 * Le e valida o arquivo de enderecos logicos: um inteiro decimal sem sinal
 * (ate 4294967295) por linha. Aceita espacos nas pontas, CRLF e linhas em branco.
 * Retorna 0 e preenche list, ou -1 e escreve a causa em err (com o numero da linha
 * quando o erro e de formato). Em caso de erro nada fica alocado.
 */
int addresses_load(const char *path, AddressList *list, char *err, size_t err_size);

/** Libera a memoria de list. */
void addresses_free(AddressList *list);

#endif
