#ifndef POLICY_H
#define POLICY_H

typedef enum {
    POLICY_FIFO,
    POLICY_LRU
} Policy;

/**
 * Converte "fifo" ou "lru" (minusculas) em Policy.
 * Retorna 0 em caso de sucesso e -1 se o texto nao for um algoritmo valido.
 */
int policy_parse(const char *text, Policy *policy);

/** Retorna o nome do algoritmo: "fifo" ou "lru". */
const char *policy_name(Policy policy);

#endif
