/*
 * program_memory.h
 *
 * Created: 2025-06-03 21:40:00
 * Author : pszczelaszkov
 */
#ifndef PROGRAM_MEMORY_H_
#define PROGRAM_MEMORY_H_

#include <avr/pgmspace.h>

inline uint8_t PROGRAM_MEMORY_read(const void* address, void* dest, uint16_t size)
{
    memcpy_P(dest, address, size);
}

inline int8_t PROGRAM_MEMORY_read_byte(const void* address)
{
    return pgm_read_byte(address);
}

inline int16_t PROGRAM_MEMORY_read_word(const void* address)
{
    return pgm_read_word(address);
}
#endif