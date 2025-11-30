/*
 * program_memory.h
 *
 * Created: 2025-06-03 21:40:00
 * Author : pszczelaszkov
 */
#ifndef PROGRAM_MEMORY_H_
#define PROGRAM_MEMORY_H_

#define PROGMEM
#include <stdint.h>
#include <string.h>

inline uint8_t PROGRAM_MEMORY_read(const void* address, void* dest, uint16_t size)
{
    memcpy(dest, address, size);
}

#endif