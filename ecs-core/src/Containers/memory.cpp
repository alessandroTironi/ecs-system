#include "Containers/memory.h"
#include "Profiling/ProfilingCore.h"
#include <cstdlib>

void* ecs::Malloc(size_t size)
{
    ecs::CycleCounter __counter_Malloc("Malloc");
    //SCOPE_CYCLE_COUNTER(Malloc)

    return malloc(size);
}

void ecs::Free(void* ptr)
{
    //SCOPE_CYCLE_COUNTER(Free)

    free(ptr);
}

void* ecs::Realloc(void* ptr, size_t size)
{
    //SCOPE_CYCLE_COUNTER(Realloc)

    return realloc(ptr, size);
}

void* ecs::Calloc(size_t count, size_t size)
{
    //SCOPE_CYCLE_COUNTER(Calloc)

    return calloc(count, size);
}