#include "vmm.h"

_Static_assert(FRAME_COUNT > 0 && FRAME_COUNT <= 32767, "FRAME_COUNT fora do intervalo");

/**
 * Escolhe o frame a ser substituido. No FIFO e o frame carregado ha mais tempo:
 * como os frames sao preenchidos em ordem, basta percorre-los em ordem circular.
 * No LRU e o frame cujo ultimo acesso e o mais antigo.
 */
static int choose_victim(Vmm *vmm)
{
    size_t victim = 0;
    size_t frame;

    if (vmm->policy == POLICY_FIFO) {
        victim = vmm->next_victim;
        vmm->next_victim = (vmm->next_victim + 1) % FRAME_COUNT;
        return (int)victim;
    }
    for (frame = 1; frame < FRAME_COUNT; frame++) {
        if (vmm->last_use[frame] < vmm->last_use[victim]) {
            victim = frame;
        }
    }
    return (int)victim;
}

void vmm_init(Vmm *vmm, BackingStore *store, Policy policy)
{
    size_t i;

    vmm->store = store;
    vmm->policy = policy;
    vmm->used_frames = 0;
    vmm->next_victim = 0;
    vmm->accesses = 0;
    vmm->page_faults = 0;
    for (i = 0; i < PAGE_COUNT; i++) {
        vmm->page_to_frame[i] = -1;
    }
    for (i = 0; i < FRAME_COUNT; i++) {
        vmm->frame_to_page[i] = -1;
        vmm->last_use[i] = 0;
    }
}

int vmm_resolve(Vmm *vmm, uint8_t page, int *frame, int *evicted, char *err, size_t err_size)
{
    int found = vmm->page_to_frame[page];

    *evicted = -1;
    if (found >= 0) {
        *frame = found;
        return 0;
    }
    if (vmm->used_frames < FRAME_COUNT) {
        found = (int)vmm->used_frames++;
    } else {
        found = choose_victim(vmm);
        *evicted = vmm->frame_to_page[found];
        vmm->page_to_frame[*evicted] = -1;
    }
    if (backing_store_read(vmm->store, page, vmm->memory[found], err, err_size) != 0) {
        return -1;
    }
    vmm->frame_to_page[found] = page;
    vmm->page_to_frame[page] = (int16_t)found;
    vmm->page_faults++;
    *frame = found;
    return 0;
}

void vmm_use(Vmm *vmm, int frame, uint8_t offset, Access *access)
{
    vmm->accesses++;
    vmm->last_use[frame] = vmm->accesses;
    access->physical = address_physical((uint32_t)frame, offset);
    access->value = (int8_t)vmm->memory[frame][offset];
    access->tlb_slot = -1;
}

int vmm_access(Vmm *vmm, LogicalAddress address, Access *access, char *err, size_t err_size)
{
    int frame;
    int evicted;

    if (vmm_resolve(vmm, address.page, &frame, &evicted, err, err_size) != 0) {
        return -1;
    }
    vmm_use(vmm, frame, address.offset, access);
    return 0;
}
