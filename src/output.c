#include "output.h"

void output_write_access(FILE *out, uint16_t virtual_address, const Access *access)
{
    fprintf(out, "Virtual address: %u Physical address: %u Value: %d\n", (unsigned)virtual_address,
            (unsigned)access->physical, (int)access->value);
}

void output_write_summary(FILE *out, const Vmm *vmm)
{
    fprintf(out, "Number of Translated Addresses = %zu\n", vmm->accesses);
    fprintf(out, "Page Faults = %zu\n", vmm->page_faults);
    fprintf(out, "Page Fault Rate = %.3f\n", (double)vmm->page_faults / (double)vmm->accesses);
}
