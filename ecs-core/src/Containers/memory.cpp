#include "Containers/memory.h"
#include <cstdlib>

using namespace ecs;
using namespace ecs::mem;

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

#include "Containers/PoolMemoryAllocator.h"
#include "Containers/memory.h"
#include <cassert>
#include <cstring>

using namespace ecs;

bucket_t::bucket_t(size_t inBlockSize, size_t inBlockCount)
    : blockSize{inBlockSize}, blockCount{inBlockCount}
{
    const size_t dataSize = blockSize * blockCount;
    m_data = static_cast<std::byte*>(Malloc(dataSize));
    assert(m_data != nullptr);
    const size_t ledgerSize = 1 + ((blockCount - 1) / 8);
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

size_t bucket_t::find_contiguous_blocks(size_t n) const noexcept 
{
    for (size_t blockIdx = 0; blockIdx < blockCount; ++blockIdx)
    {
        if (!is_block_in_use(blockIdx))
        {
            size_t firstFreeBlockIdx = blockIdx;
            for (blockIdx = blockIdx + 1; blockIdx < blockCount; ++blockIdx)
            {
                const size_t numFreeBlocks = blockIdx - firstFreeBlockIdx;
                if (numFreeBlocks >= n)
                {
                    return firstFreeBlockIdx;
                }

                if (is_block_in_use(blockIdx))
                {
                    break;
                }
            }
        }
    }

    return blockCount;
}

bool bucket_t::is_block_in_use(size_t index) const noexcept 
{
    assert(index < 1 + ((blockCount - 1) / 8));

    const size_t byteIndex = index / 8;
    const size_t byteOffset = 8 - (index - (byteIndex * 8));
    return (m_ledger[byteIndex] & std::byte(0b00000001 << byteOffset)) > std::byte(0);
}

void bucket_t::set_blocks_in_use(size_t index, size_t n) noexcept
{
    assert(index < 1 + ((blockCount - 1) / 8));

    const size_t byteIndex = index / 8;
    const size_t byteOffset = 8 - (index - (byteIndex * 8));
    m_ledger[byteIndex] |= std::byte(0b00000001 << byteOffset); 
}

void bucket_t::set_blocks_free(size_t index, size_t n) noexcept
{
    assert(index < 1 + ((blockCount - 1) / 8));

    const size_t byteIndex = index / 8;
    const size_t byteOffset = 8 - (index - (byteIndex * 8));
    m_ledger[byteIndex] &= std::byte(~(0b00000001 << byteOffset)); 
}