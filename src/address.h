#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdint.h>

#define OFFSET_BITS 8u
#define PAGE_SIZE 256u
#define PAGE_COUNT 256u
#define ADDRESS_MASK 0xFFFFu
#define OFFSET_MASK 0xFFu

typedef struct {
    uint16_t address;
    uint8_t page;
    uint8_t offset;
} LogicalAddress;

/**
 * Mantem os 16 bits da direita de um endereco logico e os separa em numero
 * da pagina (8 bits mais altos) e deslocamento (8 bits mais baixos).
 */
LogicalAddress address_split(uint32_t logical);

#endif
