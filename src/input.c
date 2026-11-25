#include "input.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define INITIAL_CAPACITY 1024

typedef struct {
    AddressList *list;
    size_t capacity;
    size_t line;
    uint64_t value;
    int has_digits;
    int number_ended;
} Parser;

/** Acrescenta value ao final da lista, dobrando a capacidade quando necessario. */
static int list_push(Parser *parser, uint32_t value)
{
    AddressList *list = parser->list;

    if (list->count == parser->capacity) {
        size_t capacity;
        uint32_t *grown;

        if (parser->capacity > SIZE_MAX / (2 * sizeof *grown)) {
            return -1;
        }
        capacity = parser->capacity * 2;
        grown = realloc(list->values, capacity * sizeof *grown);
        if (grown == NULL) {
            return -1;
        }
        list->values = grown;
        parser->capacity = capacity;
    }
    list->values[list->count++] = value;
    return 0;
}

/** Fecha a linha atual: guarda o numero lido (se houver) e prepara a proxima linha. */
static int end_line(Parser *parser, char *err, size_t err_size)
{
    if (parser->has_digits && list_push(parser, (uint32_t)parser->value) != 0) {
        snprintf(err, err_size, "memoria insuficiente");
        return -1;
    }
    parser->line++;
    parser->value = 0;
    parser->has_digits = 0;
    parser->number_ended = 0;
    return 0;
}

/** Escreve em out uma forma legivel do caractere c, inclusive os nao imprimiveis. */
static void describe_char(int c, char *out, size_t size)
{
    if (c >= 32 && c <= 126) {
        snprintf(out, size, "'%c'", c);
    } else {
        snprintf(out, size, "0x%02X", (unsigned)c);
    }
}

/** Percorre o arquivo caractere a caractere validando o formato e preenchendo a lista. */
static int parse_stream(FILE *file, Parser *parser, char *err, size_t err_size)
{
    int c;

    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            if (end_line(parser, err, err_size) != 0) {
                return -1;
            }
        } else if (c == ' ' || c == '\t' || c == '\r') {
            if (parser->has_digits) {
                parser->number_ended = 1;
            }
        } else if (c >= '0' && c <= '9') {
            if (parser->number_ended) {
                snprintf(err, err_size, "linha %zu: mais de um valor na mesma linha", parser->line);
                return -1;
            }
            parser->value = parser->value * 10 + (uint64_t)(c - '0');
            parser->has_digits = 1;
            if (parser->value > UINT32_MAX) {
                snprintf(err, err_size, "linha %zu: valor fora do intervalo (maximo %lu)",
                         parser->line, (unsigned long)UINT32_MAX);
                return -1;
            }
        } else {
            char shown[16];

            describe_char(c, shown, sizeof shown);
            snprintf(err, err_size,
                     "linha %zu: caractere invalido %s (esperado um inteiro decimal sem sinal)",
                     parser->line, shown);
            return -1;
        }
    }
    if (ferror(file)) {
        snprintf(err, err_size, "falha ao ler o arquivo de enderecos: %s", strerror(errno));
        return -1;
    }
    if (end_line(parser, err, err_size) != 0) {
        return -1;
    }
    if (parser->list->count == 0) {
        snprintf(err, err_size, "o arquivo de enderecos nao contem nenhum endereco");
        return -1;
    }
    return 0;
}

int addresses_load(const char *path, AddressList *list, char *err, size_t err_size)
{
    struct stat info;
    Parser parser;
    FILE *file;
    int status;

    list->values = NULL;
    list->count = 0;

    if (stat(path, &info) != 0) {
        if (errno == ENOENT) {
            snprintf(err, err_size, "arquivo de enderecos nao encontrado: %s", path);
        } else {
            snprintf(err, err_size, "nao foi possivel acessar '%s': %s", path, strerror(errno));
        }
        return -1;
    }
    if (!S_ISREG(info.st_mode)) {
        snprintf(err, err_size, "'%s' nao e um arquivo regular", path);
        return -1;
    }
    file = fopen(path, "r");
    if (file == NULL) {
        snprintf(err, err_size, "nao foi possivel abrir '%s': %s", path, strerror(errno));
        return -1;
    }

    list->values = malloc(INITIAL_CAPACITY * sizeof *list->values);
    if (list->values == NULL) {
        fclose(file);
        snprintf(err, err_size, "memoria insuficiente");
        return -1;
    }
    parser.list = list;
    parser.capacity = INITIAL_CAPACITY;
    parser.line = 1;
    parser.value = 0;
    parser.has_digits = 0;
    parser.number_ended = 0;

    status = parse_stream(file, &parser, err, err_size);
    fclose(file);
    if (status != 0) {
        addresses_free(list);
        return -1;
    }
    return 0;
}

void addresses_free(AddressList *list)
{
    free(list->values);
    list->values = NULL;
    list->count = 0;
}
