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
    const size_t dataSize = blockSize * blockCount * numInstances;
    std::byte* data = static_cast<std::byte*>(Malloc(dataSize));
    assert(data != nullptr);
    std::memset(data, 0, dataSize);

    bool isMemoryBlockStart = true;
    for (size_t i = 0; i < numInstances; ++i)
    {
        std::byte* chunkBegin = data + blockCount * i;
        m_data.emplace_back(chunkBegin, isMemoryBlockStart);
        isMemoryBlockStart = false;
    }

    m_memoryTracker.AddFreeBlock((m_data.size() - numInstances) * blockCount, 
        numInstances * blockCount);
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

    std::optional<size_t> optIndex = m_memoryTracker.FindAndRemoveFreeBlock(n);
    if (!optIndex.has_value())
    {
        void* ptr = Malloc(n * blockSize);
        m_fallbackAllocations.push_back(ptr);
        return ptr;
    }

    const size_t index = *optIndex;
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
    m_memoryTracker.AddFreeBlock(index, n);
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

bool bucket_container_t::is_block_in_use(size_t index) const noexcept 
{
    return !m_memoryTracker.IsBlockFree(index);
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