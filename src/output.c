#include "output.h"

void output_write_access(FILE *out, uint16_t virtual_address, const Access *access)
{
    if (USE_TLB) {
        fprintf(out, "Virtual address: %u TLB: %d Physical address: %u Value: %d\n", (unsigned)virtual_address,
                access->tlb_slot, (unsigned)access->physical, (int)access->value);
    } else {
        fprintf(out, "Virtual address: %u Physical address: %u Value: %d\n", (unsigned)virtual_address,
                (unsigned)access->physical, (int)access->value);
    }
}

void output_write_summary(FILE *out, const Mmu *mmu)
{
    double total = (double)mmu->vmm.accesses;

    fprintf(out, "Number of Translated Addresses = %zu\n", mmu->vmm.accesses);
    fprintf(out, "Page Faults = %zu\n", mmu->vmm.page_faults);
    fprintf(out, "Page Fault Rate = %.3f\n", (double)mmu->vmm.page_faults / total);
    if (USE_TLB) {
        fprintf(out, "TLB Hits = %zu\n", mmu->tlb.hits);
        fprintf(out, "TLB Hit Rate = %.3f\n", (double)mmu->tlb.hits / total);
    }
}
