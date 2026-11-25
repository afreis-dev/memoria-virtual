#include "tlb.h"

#include <pthread.h>

typedef struct {
    const TlbEntry *entry;
    uint8_t page;
    int match;
} SearchTask;

/** Corpo de cada thread: compara a pagina buscada com a entrada que lhe coube. */
static void *search_entry(void *arg)
{
    SearchTask *task = arg;

    task->match = task->entry->page == task->page;
    return NULL;
}

/** Busca sequencial: percorre as entradas em ordem e devolve a primeira que casa. */
static int find_sequential(const Tlb *tlb, uint8_t page)
{
    size_t i;

    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].page == page) {
            return (int)i;
        }
    }
    return -1;
}

/**
 * Busca paralela: uma thread por entrada. Depois de juntar todas, devolve a
 * primeira entrada que casou. Se uma thread nao puder ser criada, a comparacao
 * daquela entrada e feita pela propria thread chamadora.
 */
static int find_threaded(const Tlb *tlb, uint8_t page)
{
    pthread_t threads[TLB_SIZE];
    SearchTask tasks[TLB_SIZE];
    int started[TLB_SIZE];
    int slot = -1;
    size_t i;

    for (i = 0; i < TLB_SIZE; i++) {
        tasks[i].entry = &tlb->entries[i];
        tasks[i].page = page;
        tasks[i].match = 0;
        started[i] = pthread_create(&threads[i], NULL, search_entry, &tasks[i]) == 0;
        if (!started[i]) {
            search_entry(&tasks[i]);
        }
    }
    for (i = 0; i < TLB_SIZE; i++) {
        if (started[i]) {
            pthread_join(threads[i], NULL);
        }
        if (tasks[i].match && slot < 0) {
            slot = (int)i;
        }
    }
    return slot;
}

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
    return USE_THREADS ? find_threaded(tlb, page) : find_sequential(tlb, page);
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
