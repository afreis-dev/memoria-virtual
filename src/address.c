#include "address.h"

LogicalAddress address_split(uint32_t logical)
{
    LogicalAddress result;

    result.address = (uint16_t)(logical & ADDRESS_MASK);
    result.page = (uint8_t)(result.address >> OFFSET_BITS);
    result.offset = (uint8_t)(result.address & OFFSET_MASK);
    return result;
}
