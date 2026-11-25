#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>
#include <stdio.h>

#include "vmm.h"

/** Escreve em out uma linha de resultado: endereco virtual, endereco fisico e valor lido. */
void output_write_access(FILE *out, uint16_t virtual_address, const Access *access);

/** Escreve em out o resumo: enderecos traduzidos, page faults e taxa de page faults. */
void output_write_summary(FILE *out, const Vmm *vmm);

#endif
