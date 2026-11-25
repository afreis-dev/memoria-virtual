#include "address.h"

LogicalAddress address_split(uint32_t logical)
{
    LogicalAddress result;

    result.address = (uint16_t)(logical & ADDRESS_MASK);
    result.page = (uint8_t)(result.address >> OFFSET_BITS);
    result.offset = (uint8_t)(result.address & OFFSET_MASK);
    return result;
}

uint32_t address_physical(uint32_t frame, uint8_t offset)
{
    return (frame << OFFSET_BITS) | offset;
}
