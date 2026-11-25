#include "mmu.h"

void mmu_init(Mmu *mmu, BackingStore *store, Policy page_policy, Policy tlb_policy)
{
    vmm_init(&mmu->vmm, store, page_policy);
    tlb_init(&mmu->tlb, tlb_policy);
}

int mmu_translate(Mmu *mmu, LogicalAddress address, Access *access, char *err, size_t err_size)
{
    int slot;
    int frame;
    int evicted;

    if (!USE_TLB) {
        return vmm_access(&mmu->vmm, address, access, err, err_size);
    }
    slot = tlb_find(&mmu->tlb, address.page);
    if (slot >= 0) {
        vmm_use(&mmu->vmm, mmu->tlb.entries[slot].frame, address.offset, access);
        tlb_hit(&mmu->tlb, slot, mmu->vmm.accesses);
        access->tlb_slot = slot;
        return 0;
    }
    if (vmm_resolve(&mmu->vmm, address.page, &frame, &evicted, err, err_size) != 0) {
        return -1;
    }
    if (evicted >= 0) {
        tlb_invalidate(&mmu->tlb, (uint8_t)evicted);
    }
    vmm_use(&mmu->vmm, frame, address.offset, access);
    access->tlb_slot = tlb_insert(&mmu->tlb, address.page, frame, mmu->vmm.accesses);
    return 0;
}
