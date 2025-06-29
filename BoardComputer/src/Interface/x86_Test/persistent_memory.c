#include "persistent_memory.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
/*
    For test env STORAGE does not have fixed size
*/
static uint8_t* storage;
static uint16_t storage_size;

static inline void prepare_storage(uint16_t size)
{
    if(size > storage_size)
    {
        if(storage != 0x0)
            storage = realloc(storage,size);
        else
            storage = malloc(size);

        storage_size = size;
    }
}

uint8_t PERSISTENT_MEMORY_write(const uint32_t address, void* src, uint16_t size)
{
    prepare_storage(address + size);
    void* destination = &storage[address];
    assert(destination);
    assert((address + size) <= storage_size);
    memcpy(destination, src, size);
    return size;
}

uint8_t PERSISTENT_MEMORY_read(const uint32_t address, void* dest, uint16_t size)
{
    prepare_storage(address + size);
    void* src = &storage[address];
    assert(src);
    assert((address + size) <= storage_size);
    memcpy(dest, src, size);
    return size;
}