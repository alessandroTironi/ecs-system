#include "Containers/MemoryBlocksFreeList.h"
#include <limits>
#include <cassert>

using namespace ecs::memory_pool;

memory_blocks_free_list::block_t memory_blocks_free_list::block_t::merge_blocks(const block_t& b1, const block_t& b2, size_t newIndex)
{
    assert(b1.last_index() + 1 == b2.first_index() && "Blocks are not adjacent.");
    return block_t(newIndex, b1.first_index(), b1.size() + b2.size(), b2.m_next, b1.m_prev);
}

memory_blocks_free_list::memory_blocks_free_list()
{
    m_list = static_cast<block_t*>(Malloc(m_capacity * sizeof(block_t)));
}

memory_blocks_free_list::~memory_blocks_free_list()
{
    Free(m_list);
}

memory_blocks_free_list::block_t& memory_blocks_free_list::add(const size_t index, const size_t blockSize)
{
    assert(blockSize > 0 && "Block size must be greater than zero.");

    if (m_size == 0)
    {
        m_firstBlock = 0;
        m_list[m_size++] = block_t(0, index, blockSize);
        onBlockAdded.broadcast(0, blockSize);
        return m_list[0];
    }

    block_t newBlock = block_t(std::nullopt, index, blockSize);
    block_t& firstBlock = m_list[m_firstBlock.value()];
    if (index < firstBlock.first_index())
    {
        // insert as new head, possibly merging with current head 

        if (newBlock.last_index() + 1 == firstBlock.first_index())
        {
            // merge with current head
            const size_t originalSize = firstBlock.size();
            firstBlock = block_t::merge_blocks(newBlock, firstBlock, m_firstBlock.value());
            onBlockModified.broadcast(m_firstBlock.value(), firstBlock.size(), originalSize);
            return m_list[m_firstBlock.value()];
        }
        else if (newBlock.last_index() > firstBlock.first_index())
        {
            // overlapping blocks: merge with current head
            const size_t originalSize = firstBlock.size();
            m_list[m_firstBlock.value()] = block_t(m_firstBlock.value(), index, 
                firstBlock.last_index() - newBlock.first_index() + 1, 
                firstBlock.next(), std::nullopt);
            onBlockModified.broadcast(m_firstBlock.value(), m_list[m_firstBlock.value()].size(), originalSize);
            return m_list[m_firstBlock.value()];
        }
        else
        {
            // no merge possible, insert as new head 
            if (m_size >= m_capacity)
            {
                m_capacity *= 2;
                m_list = static_cast<block_t*>(Realloc(m_list, m_capacity * sizeof(block_t)));
            }

            newBlock.set_block_index(m_size);
            newBlock.set_next(m_firstBlock);
            firstBlock.set_prev(m_size);
            m_list[m_size] = newBlock;
            m_firstBlock = m_size; 
            onBlockAdded.broadcast(m_size, newBlock.size());
            return m_list[m_size++];
        }
    }

    // Normal case: insert in the middle or at the end
    std::optional<size_t> currentIdx = m_firstBlock.value();
    std::optional<size_t> previousIdx = std::nullopt;

    while (currentIdx.has_value() && m_list[currentIdx.value()].first_index() < index)
    {
        previousIdx = currentIdx; 
        currentIdx = m_list[currentIdx.value()].next();
    }

    // Check for merges with previous block
    if (previousIdx.has_value())
    {
        block_t& previousBlock = m_list[previousIdx.value()];
        if (previousBlock.last_index() == index - 1)
        {
            const size_t originalSize = previousBlock.size();
            m_list[previousIdx.value()] = block_t(previousIdx.value(), previousBlock.first_index(),
                previousBlock.size() + blockSize, previousBlock.next(), 
                previousBlock.prev());

            // check if the merge affects also the next block
            if (previousBlock.next().has_value())
            {
                block_t nextBlock = m_list[previousBlock.next().value()];
                if (nextBlock.first_index() - 1 == previousBlock.last_index())
                {
                    const size_t toDeleteIndex = previousBlock.next().value();
                    
                    previousBlock = block_t::merge_blocks(previousBlock, nextBlock, previousIdx.value());
                    onBlockModified.broadcast(previousIdx.value(), previousBlock.size(), 
                        originalSize);
                    remove_block(m_list[toDeleteIndex]);
                }
                else
                {
                    onBlockModified.broadcast(previousIdx.value(), previousBlock.size(), 
                        originalSize);
                }
            }
            else
            {
                onBlockModified.broadcast(previousIdx.value(), previousBlock.size(), 
                    originalSize);
            }

            return previousBlock;
        }
    }

    // Check for merges with next block
    if (currentIdx.has_value())
    {
        block_t& currentBlock = m_list[currentIdx.value()];
        if (index + blockSize == currentBlock.first_index())
        {
            const size_t originalSize = currentBlock.size();
            currentBlock = block_t(currentIdx.value(), index, currentBlock.size() + blockSize, 
                currentBlock.next(), currentBlock.prev());
            onBlockModified.broadcast(currentIdx.value(), currentBlock.size(), originalSize);
            return currentBlock;
        }
    }

    // No merging needed, insert between prev and current
    if (m_size >= m_capacity)
    {
        m_capacity *= 2;
        m_list = static_cast<block_t*>(Realloc(m_list, m_capacity * sizeof(block_t)));
    }

    newBlock.set_block_index(m_size);
    newBlock.set_next(!currentIdx.has_value() || *currentIdx == m_size? std::nullopt : currentIdx);
    if (newBlock.next().has_value())
    {
        m_list[newBlock.next().value()].set_prev(m_size);
    }
    
    newBlock.set_prev(!previousIdx.has_value() || *previousIdx == m_size? std::nullopt : previousIdx);
    if (newBlock.prev().has_value())
    {
        m_list[newBlock.prev().value()].set_next(m_size);
    }

    m_list[m_size] = newBlock;
    assert(!m_list[m_size].has_loops());

    onBlockAdded.broadcast(m_size, newBlock.size());
    return m_list[m_size++];
}

void memory_blocks_free_list::remove_block(size_t blockIndex)
{
    assert(blockIndex < m_size && "Block index out of bounds.");
    
    // First, handle connections for the block being removed
    const block_t& block = m_list[blockIndex];
    if (blockIndex == m_firstBlock.value())
    {
        m_firstBlock = block.next();
    }
    if (block.prev().has_value())
    {
        m_list[block.prev().value()].set_next(block.next());
    }
    if (block.next().has_value())
    {
        m_list[block.next().value()].set_prev(block.prev());
    }

    onBlockRemoved.broadcast(blockIndex, block.size());
    
    // If it's not the last block, we need to move the last block to this position
    if (blockIndex != m_size - 1)
    {
        block_t& lastBlock = m_list[m_size - 1];
        
        // Update references to the last block before moving it
        if (lastBlock.prev().has_value())
        {
            m_list[lastBlock.prev().value()].set_next(blockIndex);
        }
        if (lastBlock.next().has_value())
        {
            m_list[lastBlock.next().value()].set_prev(blockIndex);
        }
        
        // If the last block was the first in the list, update m_firstBlock
        if (m_firstBlock.value() == m_size - 1)
        {
            m_firstBlock = blockIndex;
        }
        
        // Copy the last block to the position of the removed block
        m_list[blockIndex] = block_t(blockIndex, lastBlock.first_index(),
            lastBlock.size(), lastBlock.next(), lastBlock.prev());
        onBlockMoved.broadcast(blockIndex, m_size - 1);
    }
    
    m_size -= 1;
    assert(!m_list[blockIndex].has_loops());
    assert(m_size == 0 || m_firstBlock.value() < m_size);

   
}

void memory_blocks_free_list::remove_block(const block_t& block)
{
    remove_block(calculate_block_index(block));
}

std::optional<size_t> memory_blocks_free_list::find_and_remove(size_t numBlocks)
{
    if (m_size == 0)
    {
        return std::nullopt;
    }

    std::optional<size_t> bestIdx = std::nullopt;
    std::optional<size_t> minWaste = std::nullopt;
    for (size_t i = 0; i < m_size; ++i)
    {
        block_t& block = m_list[i];
        if (block.size() >= numBlocks)
        {
            if (block.size() == numBlocks)
            {
                bestIdx = i;
                minWaste = 0;
                break;
            }
            else if (block.size() - numBlocks < minWaste.value_or(std::numeric_limits<size_t>::max()))
            {
                minWaste = block.size() - numBlocks;
                bestIdx = i;
            }
        }
    }

    if (bestIdx.has_value())
    {
        block_t& block = m_list[bestIdx.value()];
        const size_t originalIndex = block.first_index();

        if (minWaste.value() > 0)
        {
            const size_t originalSize = block.size();
            block = block_t(bestIdx.value(), block.first_index() + numBlocks, 
                originalSize - numBlocks, block.next(), block.prev());
            onBlockModified.broadcast(bestIdx.value(), block.size(), originalSize);
            return originalIndex;
        }
        else
        {
            remove_block(bestIdx.value());
            return originalIndex;
        }
    }

    return std::nullopt;
}

bool ecs::memory_pool::memory_blocks_free_list::is_block_free(size_t index) const noexcept
{
    for (std::optional<size_t> i = m_firstBlock; i.has_value(); 
        i = m_list[i.value()].next()) 
    {
        const block_t& block = m_list[i.value()];
        if (block.first_index() > index)
        {
            return false;
        }

        if (block.first_index() <= index && block.last_index() >= index)
        {
            return true;
        }
    }

    return false;
}

FreeMemoryTracker::FreeMemoryTracker()
{
    m_freeList.onBlockAdded += BlockAddedDelegate::create([&](size_t index, size_t size)
    {
        OnBlockAdded(index, size);
    });

    m_freeList.onBlockModified += BlockModifiedDelegate::create([&](size_t index, size_t newSize, size_t oldSize)
    {
        OnBlockModified(index, newSize, oldSize);
    });

    m_freeList.onBlockMoved += BlockMovedDelegate::create([&](size_t newIndex, size_t oldIndex)
    {
        OnBlockMoved(newIndex, oldIndex);
    });

    m_freeList.onBlockRemoved += BlockRemovedDelegate::create([&](size_t index, size_t size)
    {
        OnBlockRemoved(index, size);
    });
}

void FreeMemoryTracker::AddFreeBlock(size_t index, size_t size)
{
    m_freeList.add(index, size);
}

std::optional<size_t> FreeMemoryTracker::FindAndRemoveFreeBlock(size_t numBlocks)
{
    return m_freeList.find_and_remove(numBlocks);
}

bool FreeMemoryTracker::IsBlockFree(size_t index) const noexcept
{
    return m_freeList.is_block_free(index);
}

void FreeMemoryTracker::OnBlockAdded(size_t index, size_t size)
{
    memory_blocks_free_list::block_t& addedBlock = m_freeList.list_array()[index];
    addedBlock.set_next_with_same_size(m_freeBlocksSizeMap[size]);
    m_freeBlocksSizeMap[size] = index;
}

void FreeMemoryTracker::OnBlockRemoved(size_t index, size_t size)
{
    std::optional<size_t> previousIdx = std::nullopt;
    std::optional<size_t> currentIdx = m_freeBlocksSizeMap[size];

    while (currentIdx != index && currentIdx != std::nullopt)
    {
        memory_blocks_free_list::block_t& block = m_freeList.list_array()[currentIdx.value()];
        previousIdx = currentIdx;
        currentIdx = block.next_with_same_size();
    }

    if (currentIdx.has_value())
    {
        memory_blocks_free_list::block_t& current = m_freeList.list_array()[currentIdx.value()];
        if (previousIdx.has_value())
        {
            memory_blocks_free_list::block_t& previous = m_freeList.list_array()[previousIdx.value()];
            previous.set_next_with_same_size(current.next_with_same_size());
        }
        else
        {
            auto optionalFirstElement = m_freeBlocksSizeMap.find(size);
            if (optionalFirstElement != m_freeBlocksSizeMap.end())
            {
                if (optionalFirstElement->second.value() == index)
                {
                    optionalFirstElement->second = current.next_with_same_size();
                }

                if (!optionalFirstElement->second.has_value())
                {
                    m_freeBlocksSizeMap.erase(size);
                }
            }
        }

        current.set_next_with_same_size(std::nullopt);
    }
}

void FreeMemoryTracker::OnBlockModified(size_t index, size_t newSize, size_t oldSize)
{
    OnBlockRemoved(index, oldSize);
    OnBlockAdded(index, newSize);
}

void FreeMemoryTracker::OnBlockMoved(size_t newIndex, size_t oldIndex)
{
    const size_t size = m_freeList.list_array()[newIndex].size();
    OnBlockRemoved(oldIndex, size);
    OnBlockAdded(newIndex, size);
}