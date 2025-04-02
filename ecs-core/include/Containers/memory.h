#pragma once 

#include <cstdint>

namespace ecs
{
    void* Malloc(size_t size);

    void Free(void* ptr);

    void* Realloc(void* ptr, size_t size);

    void* Calloc(size_t count, size_t size);
}