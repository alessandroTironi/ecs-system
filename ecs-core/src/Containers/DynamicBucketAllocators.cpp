#include "Containers/DynamicBucketAllocators.h"

#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace ecs;
using namespace ecs::memory_pool;

bucket_container_t::bucket_container_t(size_t inBlockSize, size_t inBlockCount)
    : blockSize{inBlockSize}, blockCount{inBlockCount}
{
	allocate_contiguous_bucket_instances(1);
}

bucket_container_t::~bucket_container_t()
{
    for (dynamic_bucket_t& bucketInstance : m_data)
    {
        if (bucketInstance.is_memory_block_start())
        {
            Free(bucketInstance.data);
            Free(bucketInstance.ledger);
        }
    }

    for (void* p : m_fallbackAllocations)
    {
        Free(p);
    }

    m_fallbackAllocations.clear();
}

void bucket_container_t::allocate_contiguous_bucket_instances(size_t numInstances)
{
    dynamic_bucket_t instance;
    const size_t dataSize = blockSize * blockCount * numInstances;
    std::byte* data = static_cast<std::byte*>(Malloc(dataSize));
    assert(data != nullptr);
    const size_t singleLedgerSize = (blockCount + 7) >> 3;
    const size_t ledgerSize = singleLedgerSize * numInstances;
    std::byte* ledger = static_cast<std::byte*>(Malloc(ledgerSize));
    assert(ledger != nullptr);
    std::memset(data, 0, dataSize);
    std::memset(ledger, 0, ledgerSize);

    bool isMemoryBlockStart = true;
    for (size_t i = 0; i < numInstances; ++i)
    {
        std::byte* chunkBegin = data + blockCount * i;
        std::byte* chunkLedger = ledger + singleLedgerSize * i;
        m_data.emplace_back(chunkBegin, chunkLedger, isMemoryBlockStart);
        isMemoryBlockStart = false;
    }
}

bool bucket_container_t::are_blocks_contiguous(size_t index1, size_t index2) const
{
    if (index1 + 1 != index2)
    {
        return false;
    }

    const size_t instanceIndex1 = calculate_bucket_instance_index(index1);
    const size_t instanceIndex2 = calculate_bucket_instance_index(index2);
    if (instanceIndex1 == instanceIndex2)
    {
        return true;
    }

    const dynamic_bucket_t& instance1 = m_data[instanceIndex1];
    const dynamic_bucket_t& instance2 = m_data[instanceIndex2];
    return instanceIndex1 + 1 == instanceIndex2 && instance1.data + blockCount == instance2.data;
}

void* bucket_container_t::allocate(size_t bytes) noexcept 
{
    // Calculate required number of blocks 
    const size_t n = 1 + ((bytes - 1) / blockSize);

    std::optional<size_t> optIndex = find_contiguous_blocks(n);
    if (optIndex == std::nullopt)
    {
        const size_t numBuckets = (n + blockCount - 1) / blockCount;
        allocate_contiguous_bucket_instances(numBuckets);
		optIndex = find_contiguous_blocks(n);

        if (optIndex == std::nullopt)
        {
            // fallback to regular malloc if memory to allocate is too large
            void* p = Malloc(n * sizeof(blockSize));
            m_fallbackAllocations.push_back(p);
            return p;
        }
    }

    // Update the ledger
    const size_t index = *optIndex;
    set_blocks_in_use(index, n);

	const size_t bucketInstanceIndex = calculate_bucket_instance_index(index);
	dynamic_bucket_t& bucketInstance = m_data[bucketInstanceIndex];
	const size_t localIndex = index - (bucketInstanceIndex * blockCount);
    return bucketInstance.data + (localIndex * blockSize);
}

void bucket_container_t::deallocate(void* ptr, size_t bytes) noexcept 
{
    const size_t bucketInstanceIndex = find_bucket_instance_index(ptr);
	dynamic_bucket_t& bucketInstance = m_data[bucketInstanceIndex];

    const std::byte* p = static_cast<std::byte*>(ptr);
    const size_t dist = static_cast<size_t>(p - bucketInstance.data);

    // Calculate block index
    const size_t index = dist / blockSize + (bucketInstanceIndex * blockCount);

    // Calculate the required number of blocks
    const size_t n = 1 + ((bytes - 1) / blockSize);

    // Update the ledger
    set_blocks_free(index, n);
}

bool bucket_container_t::belongs_to_this(void* ptr) const noexcept 
{
    for (const dynamic_bucket_t& bucketInstance : m_data)
    {
        const std::byte* p = static_cast<std::byte*>(ptr);
        if (p >= bucketInstance.data && p < (bucketInstance.data + (blockSize * blockCount)))
        {
            return true;
        }
    }

    return false;
}

std::optional<size_t> bucket_container_t::find_contiguous_blocks(size_t n) const noexcept 
{
    const size_t totalBlocks = blockCount * m_data.size();
    for (size_t blockIdx = 0; blockIdx < totalBlocks; ++blockIdx)
    {
        if (!is_block_in_use(blockIdx))
        {
            size_t contiguousCount = 1;
            size_t firstFreeBlockIdx = blockIdx;
            if (contiguousCount >= n)
            {
                return firstFreeBlockIdx;
            }

            const size_t lastBlockIdx = firstFreeBlockIdx + n > totalBlocks?
                totalBlocks : firstFreeBlockIdx + n;
            for (blockIdx = blockIdx + 1; blockIdx < lastBlockIdx; ++blockIdx)
            {
                if (!is_block_in_use(blockIdx) && are_blocks_contiguous(blockIdx - 1, blockIdx))
                {
                    contiguousCount += 1;
                    if (contiguousCount >= n)
                    {
                        return firstFreeBlockIdx;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    return std::nullopt;
}

bool bucket_container_t::is_block_in_use(size_t index) const noexcept 
{
    const size_t instanceIndex = calculate_bucket_instance_index(index);
    const size_t localIndex = index - (instanceIndex * blockCount);
    const size_t byteIndex = localIndex >> 3; // <- divide by 8
    const size_t bitOffset = 7 - (localIndex - (byteIndex * 8));  
    return (m_data[instanceIndex].ledger[byteIndex] & (std::byte(1) << bitOffset)) != std::byte(0);
}

void bucket_container_t::set_blocks_in_use(size_t index, size_t n) noexcept
{
    const size_t instanceIndex = calculate_bucket_instance_index(index);
    const size_t localIndex = index - (instanceIndex * blockCount);

    for (size_t i = 0; i < n; ++i)
    {    
        const size_t currentIndex = localIndex + i;
        const size_t byteIndex = currentIndex >> 3; // <- divide by 8
        const size_t bitOffset = 7 - (currentIndex - (byteIndex * 8));
        m_data[instanceIndex].ledger[byteIndex] |= (std::byte(1) << bitOffset);
    } 
}

void bucket_container_t::set_blocks_free(size_t index, size_t n) noexcept
{
    const size_t instanceIndex = calculate_bucket_instance_index(index);
    const size_t localIndex = index - (instanceIndex * blockCount);

    for (size_t i = 0; i < n; ++i)
    {
        const size_t currentIndex = localIndex + i;
        const size_t byteIndex = currentIndex >> 3; // <- divide by 8
        const size_t bitOffset = 7 - (currentIndex - (byteIndex * 8));
        m_data[instanceIndex].ledger[byteIndex] &= ~(std::byte(1) << bitOffset);
    } 
}

size_t bucket_container_t::calculate_bucket_instance_index(const size_t blockIndex) const
{
    return blockIndex / blockCount;
}

size_t bucket_container_t::find_bucket_instance_index(void* ptr) const
{
    const std::byte* p = static_cast<std::byte*>(ptr);
    for (size_t index = 0; index < m_data.size(); ++index)
    {
        const dynamic_bucket_t& instance = m_data[index];
        const std::byte* instanceEnd = instance.data + (blockCount * blockSize);
        if (p >= instance.data && p < instanceEnd)
        {
            return index;
        }
    }

    return m_data.size();
}