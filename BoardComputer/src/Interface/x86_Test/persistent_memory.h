/*
 * persistent_memory.h
 *
 * Created: 2025-06-03 21:40:00
 * Author : pszczelaszkov
 */
#ifndef PERSISTENT_MEMORY_H_
#define PERSISTENT_MEMORY_H_

#include<stdint.h>

TESTUSE uint8_t PERSISTENT_MEMORY_write(const uint32_t address, void* src, uint16_t size);
TESTUSE uint8_t PERSISTENT_MEMORY_read(const uint32_t address, void* dest, uint16_t size);

#endif