#ifndef MMU_H
#define MMU_H

#include <stddef.h>

#include "backing_store.h"
#include "policy.h"
#include "tlb.h"
#include "vmm.h"

#ifndef USE_TLB
#define USE_TLB 1
#endif

typedef struct {
    Vmm vmm;
    Tlb tlb;
} Mmu;

/** Prepara a memoria virtual e a TLB com os algoritmos de substituicao escolhidos. */
void mmu_init(Mmu *mmu, BackingStore *store, Policy page_policy, Policy tlb_policy);

/**
 * Traduz um endereco logico. Com TLB, consulta ela primeiro; em caso de miss
 * usa a tabela de paginas (carregando a pagina se preciso) e guarda a traducao
 * na TLB, removendo dela a pagina que perdeu o frame. Sem TLB, usa so a tabela.
 * Retorna 0 e preenche access (tlb_slot e a entrada usada, ou -1 sem TLB),
 * ou -1 com a causa em err.
 */
int mmu_translate(Mmu *mmu, LogicalAddress address, Access *access, char *err, size_t err_size);

#endif
