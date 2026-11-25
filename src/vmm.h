#ifndef VMM_H
#define VMM_H

#include <stddef.h>
#include <stdint.h>

#include "address.h"
#include "backing_store.h"
#include "policy.h"

#ifndef FRAME_COUNT
#define FRAME_COUNT 128u
#endif

typedef struct {
    BackingStore *store;
    Policy policy;
    int16_t page_to_frame[PAGE_COUNT];
    int16_t frame_to_page[FRAME_COUNT];
    uint8_t memory[FRAME_COUNT][PAGE_SIZE];
    size_t last_use[FRAME_COUNT];
    size_t used_frames;
    size_t next_victim;
    size_t accesses;
    size_t page_faults;
} Vmm;

typedef struct {
    uint32_t physical;
    int8_t value;
    int tlb_slot;
} Access;

/** Prepara o gerenciador: memoria fisica vazia e nenhuma pagina presente. */
void vmm_init(Vmm *vmm, BackingStore *store, Policy policy);

/**
 * Descobre em que frame esta page. Se ela nao esta na memoria fisica, carrega
 * da memoria secundaria (page fault): em um frame livre ou, com a memoria cheia,
 * no frame escolhido pelo algoritmo de substituicao (FIFO ou LRU).
 * Preenche frame e evicted (pagina que perdeu o frame, ou -1).
 * Retorna 0, ou -1 com a causa em err.
 */
int vmm_resolve(Vmm *vmm, uint8_t page, int *frame, int *evicted, char *err, size_t err_size);

/**
 * Le o byte do frame no deslocamento indicado, conta o acesso e marca o frame
 * como usado agora. Preenche o endereco fisico e o valor em access.
 */
void vmm_use(Vmm *vmm, int frame, uint8_t offset, Access *access);

/** Traduz um endereco logico consultando so a tabela de paginas (sem TLB). */
int vmm_access(Vmm *vmm, LogicalAddress address, Access *access, char *err, size_t err_size);

#endif
