#include "memory.h"
#include <cstdlib>

void* ecs::Malloc(size_t size)
{
    return malloc(size);
}

void ecs::Free(void* ptr)
{
    free(ptr);
}

void* ecs::Realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void* ecs::Calloc(size_t count, size_t size)
{
    return calloc(count, size);
}