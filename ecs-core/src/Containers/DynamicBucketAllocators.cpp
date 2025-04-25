#include "Containers/DynamicBucketAllocators.h"

#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace ecs;
using namespace ecs::memory_pool;

bucket_container_t::bucket_container_t(size_t inBlockSize, size_t inBlockCount)
    : blockSize{inBlockSize}, blockCount{inBlockCount}
{
	allocate_bucket_instance();
}

bucket_container_t::~bucket_container_t()
{
	for (dynamic_bucket_t& bucketInstance : m_data)
	{
		Free(bucketInstance.data);
		Free(bucketInstance.ledger);
	}
}

void bucket_container_t::allocate_bucket_instance()
{
    dynamic_bucket_t instance;
    const size_t dataSize = blockSize * blockCount;
    instance.data = static_cast<std::byte*>(Malloc(dataSize));
    assert(instance.data != nullptr);
    const size_t ledgerSize = (blockCount + 7) >> 3;
    instance.ledger = static_cast<std::byte*>(Malloc(ledgerSize));
    assert(instance.ledger != nullptr);
    std::memset(instance.data, 0, dataSize);
    std::memset(instance.ledger, 0, ledgerSize);

    m_data.push_back(instance);
}

void* bucket_container_t::allocate(size_t bytes) noexcept 
{
    // Calculate required number of blocks 
    const size_t n = 1 + ((bytes - 1) / blockSize);

    size_t index = find_contiguous_blocks(n);
    if (index == blockCount)
    {
        allocate_bucket_instance();
		index = find_contiguous_blocks(n);
    }

    // Update the ledger
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

size_t bucket_container_t::find_contiguous_blocks(size_t n) const noexcept 
{
    for (size_t instanceIndex = 0; instanceIndex < m_data.size(); ++instanceIndex)
    {
        const size_t startingIndex = instanceIndex * blockCount;
        const size_t finalIndex = startingIndex + blockCount;

        for (size_t blockIdx = startingIndex; blockIdx < finalIndex; ++blockIdx)
        {
            if (!is_block_in_use(blockIdx))
            {
                size_t contiguousCount = 1;  
                size_t firstFreeBlockIdx = blockIdx;
                if (contiguousCount >= n)
                {
                    return firstFreeBlockIdx;
                }

                while (blockIdx + 1 < finalIndex && !is_block_in_use(blockIdx + 1))
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
    }

    
    return blockCount;
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