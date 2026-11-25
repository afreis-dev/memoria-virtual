#include "vmm.h"

#include <stdio.h>

_Static_assert(FRAME_COUNT > 0 && FRAME_COUNT <= 32767, "FRAME_COUNT fora do intervalo");

void vmm_init(Vmm *vmm, BackingStore *store)
{
    size_t page;

    vmm->store = store;
    vmm->used_frames = 0;
    vmm->accesses = 0;
    vmm->page_faults = 0;
    for (page = 0; page < PAGE_COUNT; page++) {
        vmm->page_to_frame[page] = -1;
    }
}

int vmm_access(Vmm *vmm, LogicalAddress address, Access *access, char *err, size_t err_size)
{
    int frame = vmm->page_to_frame[address.page];
    int fault = frame < 0;

    if (fault) {
        if (vmm->used_frames >= FRAME_COUNT) {
            snprintf(err, err_size, "memoria fisica cheia e sem algoritmo de substituicao");
            return -1;
        }
        frame = (int)vmm->used_frames;
        if (backing_store_read(vmm->store, address.page, vmm->memory[frame], err, err_size) != 0) {
            return -1;
        }
        vmm->used_frames++;
        vmm->page_to_frame[address.page] = (int16_t)frame;
        vmm->page_faults++;
    }
    vmm->accesses++;
    access->page_fault = fault;
    access->physical = address_physical((uint32_t)frame, address.offset);
    access->value = (int8_t)vmm->memory[frame][address.offset];
    return 0;
}
