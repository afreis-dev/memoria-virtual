#include "tlb.h"

/**
 * Escolhe a entrada a ser ocupada. No FIFO percorre as entradas em ordem
 * circular. No LRU usa a primeira entrada livre e, sem nenhuma, a de uso mais antigo.
 */
static size_t choose_slot(Tlb *tlb)
{
    size_t slot;
    size_t i;

    if (tlb->policy == POLICY_FIFO) {
        slot = tlb->next_slot;
        tlb->next_slot = (tlb->next_slot + 1) % TLB_SIZE;
        return slot;
    }
    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].page < 0) {
            return i;
        }
    }
    slot = 0;
    for (i = 1; i < TLB_SIZE; i++) {
        if (tlb->entries[i].last_use < tlb->entries[slot].last_use) {
            slot = i;
        }
    }
    return slot;
}

void tlb_init(Tlb *tlb, Policy policy)
{
    size_t i;

    tlb->policy = policy;
    tlb->next_slot = 0;
    tlb->hits = 0;
    for (i = 0; i < TLB_SIZE; i++) {
        tlb->entries[i].page = -1;
        tlb->entries[i].frame = -1;
        tlb->entries[i].last_use = 0;
    }
}

int tlb_find(const Tlb *tlb, uint8_t page)
{
    size_t i;

    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].page == page) {
            return (int)i;
        }
    }
    return -1;
}

void tlb_hit(Tlb *tlb, int slot, size_t now)
{
    tlb->hits++;
    tlb->entries[slot].last_use = now;
}

int tlb_insert(Tlb *tlb, uint8_t page, int frame, size_t now)
{
    size_t slot = choose_slot(tlb);

    tlb->entries[slot].page = page;
    tlb->entries[slot].frame = (int16_t)frame;
    tlb->entries[slot].last_use = now;
    return (int)slot;
}

void tlb_invalidate(Tlb *tlb, uint8_t page)
{
    int slot = tlb_find(tlb, page);

    if (slot >= 0) {
        tlb->entries[slot].page = -1;
    }
}
