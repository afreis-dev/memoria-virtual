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

int vmm_access(Vmm *vmm, LogicalAddress address, Access *access, char *err, size_t err_size)
{
    int frame = vmm->page_to_frame[address.page];
    int fault = frame < 0;

    if (fault) {
        if (vmm->used_frames < FRAME_COUNT) {
            frame = (int)vmm->used_frames++;
        } else {
            frame = choose_victim(vmm);
            vmm->page_to_frame[vmm->frame_to_page[frame]] = -1;
        }
        if (backing_store_read(vmm->store, address.page, vmm->memory[frame], err, err_size) != 0) {
            return -1;
        }
        vmm->frame_to_page[frame] = address.page;
        vmm->page_to_frame[address.page] = (int16_t)frame;
        vmm->page_faults++;
    }
    vmm->accesses++;
    vmm->last_use[frame] = vmm->accesses;
    access->page_fault = fault;
    access->physical = address_physical((uint32_t)frame, address.offset);
    access->value = (int8_t)vmm->memory[frame][address.offset];
    return 0;
}
