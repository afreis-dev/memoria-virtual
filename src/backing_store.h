#ifndef BACKING_STORE_H
#define BACKING_STORE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE *file;
} BackingStore;

/**
 * Abre o arquivo da memoria secundaria. Ele deve ser um arquivo regular com
 * exatamente 256 paginas de 256 bytes. Retorna 0, ou -1 com a causa em err.
 */
int backing_store_open(BackingStore *store, const char *path, char *err, size_t err_size);

/**
 * Le a pagina indicada (256 bytes) para buffer.
 * Retorna 0, ou -1 com a causa em err.
 */
int backing_store_read(BackingStore *store, uint8_t page, uint8_t *buffer, char *err, size_t err_size);

/** Fecha o arquivo da memoria secundaria. */
void backing_store_close(BackingStore *store);

#endif
