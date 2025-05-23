#pragma once

#include <optional>
#include <cassert>
#include <unordered_map>
#include "memory.h"
#include "Core/Delegates.h"

namespace ecs
{
    namespace memory_pool
    {
        DEFINE_MULTICAST_DELEGATE(BlockRemovedDelegate, size_t, size_t);
        DEFINE_MULTICAST_DELEGATE(BlockModifiedDelegate, size_t, size_t, size_t);
        DEFINE_MULTICAST_DELEGATE(BlockMovedDelegate, size_t, size_t);
        DEFINE_MULTICAST_DELEGATE(BlockAddedDelegate, size_t, size_t);

        struct memory_blocks_free_list
        {
            struct block_t
            {
            public:
                block_t() = default;
                block_t(const std::optional<size_t> blockIndex, const size_t& inIndex, const size_t& inSize,
                    std::optional<size_t> inNext = std::nullopt, 
                    std::optional<size_t> inPrev = std::nullopt) noexcept
                    : m_blockIndex{blockIndex}
                    , m_index{inIndex}
                    , m_size{inSize}
                    , m_next{inNext}
                    , m_prev{inPrev}
                {
                    assert(!has_loops());
                }

                inline size_t first_index() const noexcept { return m_index; }
                inline size_t last_index() const noexcept { return m_index + m_size - 1; }
                inline size_t size() const noexcept { return m_size; }
                inline std::optional<size_t> next() const noexcept { return m_next; }
                inline std::optional<size_t> prev() const noexcept { return m_prev; }
                inline std::optional<size_t> next_with_same_size() const noexcept { return m_nextWithSameSize; }

                inline void set_block_index(size_t blockIndex) 
                { 
#ifdef DEBUG_BUILD
                    m_blockIndex = blockIndex; 
                    assert(!has_loops());
#endif
                }

                inline void set_next(std::optional<size_t> inNext) 
                {
                    m_next = inNext;
                    assert(!has_loops());
                }

                inline void set_prev(std::optional<size_t> inPrev)
                {
                    m_prev = inPrev;
                    assert(!has_loops());
                }

                inline void set_next_with_same_size(std::optional<size_t> inNext)
                {
                    m_nextWithSameSize = inNext;
                    assert(!has_loops());
                }

                static block_t merge_blocks(const block_t& b1, const block_t& b2, size_t newIndex);

                inline bool has_loops() const
                {
#ifdef DEBUG_BUILD
                    if (!m_blockIndex.has_value())
                        return false;
                    return m_next.has_value() && m_next.value() == *m_blockIndex 
                        || m_prev.has_value() && m_prev.value() == *m_blockIndex
                        || m_nextWithSameSize.has_value() && m_nextWithSameSize.value() == *m_blockIndex;
#else 
                    return false;
#endif
                }

            private:
                size_t m_index = 0;
                size_t m_size = 0;
                std::optional<size_t> m_next = std::nullopt;
                std::optional<size_t> m_prev = std::nullopt;
                std::optional<size_t> m_nextWithSameSize = std::nullopt;
#ifdef DEBUG_BUILD
                std::optional<size_t> m_blockIndex = std::nullopt;
#endif
            };

            memory_blocks_free_list();
            ~memory_blocks_free_list();

            block_t& add(const size_t inIndex, const size_t inSize);
            void remove_block(const block_t& block);
            void remove_block(size_t blockIndex);
            std::optional<size_t> find_and_remove(size_t numBlocks);
            bool is_block_free(size_t index) const noexcept;

            block_t& operator[](size_t index) 
            {
                assert(index < m_size && "Index out of bounds.");
                size_t blockIndex = m_firstBlock.value();
                for (size_t i = 0; i < index; ++i)
                {
                    blockIndex = m_list[blockIndex].next().value();
                }

                return m_list[blockIndex];
            }

            inline size_t size() const noexcept { return m_size; }
            inline block_t* list_array() const noexcept { return m_list; }

            BlockAddedDelegate onBlockAdded;
            BlockModifiedDelegate onBlockModified;
            BlockMovedDelegate onBlockMoved;
            BlockRemovedDelegate onBlockRemoved;
        private:
            block_t* m_list = nullptr;
            std::optional<size_t> m_firstBlock = std::nullopt;
            size_t m_size = 0;
            size_t m_capacity = 8;

            inline size_t calculate_block_index(const block_t& block)
            {
                return &block - m_list;
            }
        };

        class FreeMemoryTracker
        {
        public:
            FreeMemoryTracker();

            void AddFreeBlock(size_t index, size_t blockSize);
            std::optional<size_t> FindAndRemoveFreeBlock(size_t numBlocks);
            bool IsBlockFree(size_t index) const noexcept;

        private:
            memory_blocks_free_list m_freeList;
            std::unordered_map<size_t, std::optional<size_t>> m_freeBlocksSizeMap;

            void OnBlockAdded(size_t blockIndex, size_t blockSize);
            void OnBlockModified(size_t blockIndex, size_t newSize, size_t oldSize);
            void OnBlockMoved(size_t newIdex, size_t oldIndex);
            void OnBlockRemoved(size_t blockIndex, size_t blockSize);
        };
    }
}