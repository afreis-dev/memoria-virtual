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
    size_t used_frames;
    size_t next_victim;
    size_t accesses;
    size_t page_faults;
} Vmm;

typedef struct {
    uint32_t physical;
    int8_t value;
    int page_fault;
} Access;

/**
 * Prepara o gerenciador: memoria fisica vazia e nenhuma pagina presente.
 * Retorna 0, ou -1 com a causa em err se o algoritmo de substituicao nao for suportado.
 */
int vmm_init(Vmm *vmm, BackingStore *store, Policy policy, char *err, size_t err_size);

/**
 * Traduz um endereco logico. Se a pagina nao esta na memoria fisica, carrega
 * ela da memoria secundaria (page fault): em um frame livre, ou, com a memoria
 * cheia, no frame escolhido pelo algoritmo de substituicao.
 * Retorna 0 e preenche access, ou -1 com a causa em err.
 */
int vmm_access(Vmm *vmm, LogicalAddress address, Access *access, char *err, size_t err_size);

#endif
