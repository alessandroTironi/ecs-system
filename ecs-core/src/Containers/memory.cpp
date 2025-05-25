#include "Containers/memory.h"
#include <cstdlib>
#include <cassert>
#include <cstring>
#include "Profiler.h"

using namespace ecs;
using namespace ecs::memory_pool;

void* ecs::Malloc(size_t size)
{
    SCOPE_CYCLE_COUNTER(Malloc);
    return malloc(size);
}

void ecs::Free(void* ptr)
{
    SCOPE_CYCLE_COUNTER(Free);
    free(ptr);
}

void* ecs::Realloc(void* ptr, size_t size)
{
    SCOPE_CYCLE_COUNTER(Realloc);
    return realloc(ptr, size);
}

void* ecs::Calloc(size_t count, size_t size)
{
    SCOPE_CYCLE_COUNTER(Calloc);
    return calloc(count, size);
}

bucket_t::bucket_t(size_t inBlockSize, size_t inBlockCount)
    : blockSize{inBlockSize}, blockCount{inBlockCount}
{
    const size_t dataSize = blockSize * blockCount;
    m_data = static_cast<std::byte*>(Malloc(dataSize));
    assert(m_data != nullptr);
    const size_t ledgerSize = (blockCount + 7) >> 3;
    m_ledger = static_cast<std::byte*>(Malloc(ledgerSize));
    assert(m_ledger != nullptr);
    std::memset(m_data, 0, dataSize);
    std::memset(m_ledger, 0, ledgerSize);
}

bucket_t::~bucket_t()
{
    Free(m_data);
    Free(m_ledger);
}

void* bucket_t::allocate(size_t bytes) noexcept 
{
    // Calculate required number of blocks 
    const size_t n = 1 + ((bytes - 1) / blockSize);

    const size_t index = find_contiguous_blocks(n);
    if (index == blockCount)
    {
        return nullptr;
    }

    // Update the ledger
    set_blocks_in_use(index, n);
    return m_data + (index * blockSize);
}

void bucket_t::deallocate(void* ptr, size_t bytes) noexcept 
{
    const std::byte* p = static_cast<std::byte*>(ptr);
    const size_t dist = static_cast<size_t>(p - m_data);

    // Calculate block index
    const size_t index = dist / blockSize;

    // Calculate the required number of blocks
    const size_t n = 1 + ((bytes - 1) / blockSize);

    // Update the ledger
    set_blocks_free(index, n);
}

bool bucket_t::belongs_to_this(void* ptr) const noexcept 
{
    const std::byte* p = static_cast<std::byte*>(ptr);
    return (p >= m_data && p < (m_data + (blockSize * blockCount)));
}

size_t bucket_t::find_contiguous_blocks(size_t n) const noexcept 
{
    for (size_t blockIdx = 0; blockIdx < blockCount; ++blockIdx)
    {
        if (!is_block_in_use(blockIdx))
        {
            size_t contiguousCount = 1;  
            size_t firstFreeBlockIdx = blockIdx;
            
            while (blockIdx + 1 < blockCount && !is_block_in_use(blockIdx + 1))
            {
                contiguousCount++;
                if (contiguousCount >= n)
                {
                    return firstFreeBlockIdx;
                }
                blockIdx++;
            }
        }
    }
    return blockCount;
}

bool bucket_t::is_block_in_use(size_t index) const noexcept 
{
    const size_t byteIndex = index >> 3; // <- divide by 8
    const size_t bitOffset = 7 - (index - (byteIndex * 8));  
    return (m_ledger[byteIndex] & (std::byte(1) << bitOffset)) != std::byte(0);
}

void bucket_t::set_blocks_in_use(size_t index, size_t n) noexcept
{
    for (size_t i = 0; i < n; ++i)
    {
        const size_t currentIndex = index + i;
        const size_t byteIndex = currentIndex >> 3; // <- divide by 8
        const size_t bitOffset = 7 - (currentIndex - (byteIndex * 8));
        m_ledger[byteIndex] |= (std::byte(1) << bitOffset);
    } 
}

void bucket_t::set_blocks_free(size_t index, size_t n) noexcept
{
    for (size_t i = 0; i < n; ++i)
    {
        const size_t currentIndex = index + i;
        const size_t byteIndex = currentIndex >> 3; // <- divide by 8
        const size_t bitOffset = 7 - (currentIndex - (byteIndex * 8));
        m_ledger[byteIndex] &= ~(std::byte(1) << bitOffset);
    } 
}