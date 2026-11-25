#ifndef VMM_H
#define VMM_H

#include <stddef.h>
#include <stdint.h>

#include "address.h"
#include "backing_store.h"

#ifndef FRAME_COUNT
#define FRAME_COUNT 256u
#endif

typedef struct {
    BackingStore *store;
    int16_t page_to_frame[PAGE_COUNT];
    uint8_t memory[FRAME_COUNT][PAGE_SIZE];
    size_t used_frames;
    size_t accesses;
    size_t page_faults;
} Vmm;

typedef struct {
    uint32_t physical;
    int8_t value;
    int page_fault;
} Access;

/** Prepara o gerenciador: memoria fisica vazia e nenhuma pagina presente. */
void vmm_init(Vmm *vmm, BackingStore *store);

/**
 * Traduz um endereco logico. Se a pagina nao esta na memoria fisica, carrega
 * ela da memoria secundaria no proximo frame livre (page fault).
 * Retorna 0 e preenche access, ou -1 com a causa em err.
 */
int vmm_access(Vmm *vmm, LogicalAddress address, Access *access, char *err, size_t err_size);

#endif
