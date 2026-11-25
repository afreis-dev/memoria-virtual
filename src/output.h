#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>
#include <stdio.h>

#include "mmu.h"

/**
 * Escreve em out uma linha de resultado: endereco virtual, entrada da TLB
 * (quando ha TLB), endereco fisico e valor lido.
 */
void output_write_access(FILE *out, uint16_t virtual_address, const Access *access);

/** Escreve em out o resumo: enderecos traduzidos, page faults e, com TLB, os hits. */
void output_write_summary(FILE *out, const Mmu *mmu);

#endif
