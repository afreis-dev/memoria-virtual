#ifndef TLB_H
#define TLB_H

#include <stddef.h>
#include <stdint.h>

#include "policy.h"

#define TLB_SIZE 16u

typedef struct {
    int16_t page;
    int16_t frame;
    size_t last_use;
} TlbEntry;

typedef struct {
    TlbEntry entries[TLB_SIZE];
    Policy policy;
    size_t next_slot;
    size_t hits;
} Tlb;

/** Esvazia a TLB e define o algoritmo de substituicao das entradas. */
void tlb_init(Tlb *tlb, Policy policy);

/** Procura page na TLB. Retorna o indice da entrada ou -1 se nao estiver la. */
int tlb_find(const Tlb *tlb, uint8_t page);

/** Registra um hit na entrada slot: conta o hit e marca o uso em now. */
void tlb_hit(Tlb *tlb, int slot, size_t now);

/**
 * Guarda a traducao page -> frame na entrada escolhida pelo algoritmo
 * (FIFO: a mais antiga; LRU: uma livre ou a menos usada recentemente).
 * Retorna o indice da entrada usada.
 */
int tlb_insert(Tlb *tlb, uint8_t page, int frame, size_t now);

/** Remove da TLB a entrada de page, se existir. */
void tlb_invalidate(Tlb *tlb, uint8_t page);

#endif
