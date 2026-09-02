#include "persistent_memory.h"
#include <avr/eeprom.h>

uint8_t PERSISTENT_MEMORY_write(const uint32_t address, const void* src, uint16_t size)
{
    if (address > E2END || (uint32_t)size > (E2END + 1UL) - address)
        return 0;
    eeprom_update_block(src, (void *)(uintptr_t)address, size);
    eeprom_busy_wait();
    return (uint8_t)size;
}

uint8_t PERSISTENT_MEMORY_read(const uint32_t address, void* dest, uint16_t size)
{
    if (address > E2END || (uint32_t)size > (E2END + 1UL) - address)
        return 0;
    eeprom_read_block(dest, (const void *)(uintptr_t)address, size);
    return (uint8_t)size;
}
