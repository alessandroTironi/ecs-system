#include "Containers/MemoryBlocksFreeList.h"
#include <limits>
#include <cassert>

using namespace ecs::memory_pool;

memory_blocks_free_list::block_t memory_blocks_free_list::block_t::merge_blocks(const block_t& b1, const block_t& b2)
{
    assert(b1.last_index() + 1 == b2.first_index() && "Blocks are not adjacent.");
    return block_t(b1.first_index(), b1.size() + b2.size(), b2.next, b1.prev);
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
        m_list[m_size++] = block_t(index, blockSize);
        return m_list[0];
    }

    block_t newBlock = block_t(index, blockSize);
    block_t& firstBlock = m_list[m_firstBlock.value()];
    if (index < firstBlock.first_index())
    {
        // insert as new head, possibly merging with current head 

        if (newBlock.last_index() + 1 == firstBlock.first_index())
        {
            // merge with current head
            firstBlock = block_t::merge_blocks(newBlock, firstBlock);
            return m_list[m_firstBlock.value()];
        }
        else if (newBlock.last_index() > firstBlock.first_index())
        {
            // overlapping blocks: merge with current head
            m_list[m_firstBlock.value()] = block_t(index, 
                firstBlock.last_index() - newBlock.first_index() + 1, 
                firstBlock.next, std::nullopt);
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

            newBlock.next = m_firstBlock;
            firstBlock.prev = m_size;
            m_list[m_size] = newBlock;
            m_firstBlock = m_size; 
            return m_list[m_size++];
        }
    }

    // Normal case: insert in the middle or at the end
    std::optional<size_t> currentIdx = m_firstBlock.value();
    std::optional<size_t> previousIdx = std::nullopt;

    while (currentIdx.has_value() && m_list[currentIdx.value()].first_index() < index)
    {
        previousIdx = currentIdx; 
        currentIdx = m_list[currentIdx.value()].next;
    }

    // Check for merges with previous block
    if (previousIdx.has_value())
    {
        block_t& previousBlock = m_list[previousIdx.value()];
        if (previousBlock.last_index() == index - 1)
        {
            m_list[previousIdx.value()] = block_t(previousBlock.first_index(),
                previousBlock.size() + blockSize, previousBlock.next, 
                previousBlock.prev);

            // check if the merge affects also the next block
            if (previousBlock.next.has_value())
            {
                block_t nextBlock = m_list[previousBlock.next.value()];
                if (nextBlock.first_index() - 1 == previousBlock.last_index())
                {
                    const size_t toDeleteIndex = previousBlock.next.value();
                    previousBlock = block_t::merge_blocks(previousBlock, nextBlock);
                    remove_block(m_list[toDeleteIndex]);
                }
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
            currentBlock = block_t(index, currentBlock.size() + blockSize, 
                currentBlock.next, currentBlock.prev);
            assert(!currentBlock.has_loops(currentIdx.value()));

            return currentBlock;
        }
    }

    // No merging needed, insert between prev and current
    if (m_size >= m_capacity)
    {
        m_capacity *= 2;
        m_list = static_cast<block_t*>(Realloc(m_list, m_capacity * sizeof(block_t)));
    }

    newBlock.next = currentIdx;
    if (newBlock.next.has_value())
    {
        m_list[newBlock.next.value()].prev = m_size;
    }
    
    newBlock.prev = previousIdx;
    if (newBlock.prev.has_value())
    {
        m_list[newBlock.prev.value()].next = m_size;
    }

    m_list[m_size] = newBlock;
    assert(!m_list[m_size].has_loops(m_size));
    return m_list[m_size++];
}

void memory_blocks_free_list::remove_block(size_t blockIndex)
{
    assert(blockIndex < m_size && "Block index out of bounds.");
    const block_t& block = m_list[blockIndex];

    if (blockIndex == m_firstBlock.value())
    {
        m_firstBlock = block.next;
    }

    if (block.prev.has_value())
    {
        m_list[block.prev.value()].next = block.next;
    }

    if (block.next.has_value())
    {
        m_list[block.next.value()].prev = block.prev;
    }

    block_t& lastBlock = m_list[m_size - 1];
    if (lastBlock.prev.has_value())
    {
        block_t& blockBeforeLast = m_list[lastBlock.prev.value()];
        blockBeforeLast.next = (blockIndex == m_size - 1)? 
            std::nullopt : std::optional<size_t>(blockIndex);
    }

    m_list[blockIndex] = block_t(lastBlock.first_index(), 
        lastBlock.size(), lastBlock.next, lastBlock.prev);

    m_size -= 1;
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
            block = block_t(block.first_index() + numBlocks, 
                originalSize - numBlocks, block.next, block.prev);
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
        i = m_list[i.value()].next) 
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

void ecs::memory_pool::FreeMemoryTracker::AddFreeBlock(size_t index, size_t size)
{
    m_freeList.add(index, size);
}

std::optional<size_t> ecs::memory_pool::FreeMemoryTracker::FindAndRemoveFreeBlock(size_t numBlocks)
{
    return m_freeList.find_and_remove(numBlocks);
}

bool ecs::memory_pool::FreeMemoryTracker::IsBlockFree(size_t index) const noexcept
{
    return m_freeList.is_block_free(index);
}