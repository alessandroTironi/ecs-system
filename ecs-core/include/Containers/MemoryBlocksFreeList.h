#pragma once

#include <optional>
#include <cassert>
#include <unordered_map>
#include "memory.h"

namespace ecs
{
    namespace memory_pool
    {
        struct memory_blocks_free_list
        {
            struct block_t
            {
            public:
                block_t() = default;
                block_t(const size_t& inIndex, const size_t& inSize,
                    std::optional<size_t> inNext = std::nullopt, 
                    std::optional<size_t> inPrev = std::nullopt) noexcept
                    : m_index{inIndex}
                    , m_size{inSize}
                    , next{inNext}
                    , prev{inPrev}
                {}

                std::optional<size_t> next = std::nullopt;
                std::optional<size_t> prev = std::nullopt;

                inline size_t first_index() const noexcept { return m_index; }
                inline size_t last_index() const noexcept { return m_index + m_size - 1; }
                inline size_t size() const noexcept { return m_size; }

                static block_t merge_blocks(const block_t& b1, const block_t& b2);

                inline bool has_loops(const size_t index) const
                {
                    return next.has_value() && next.value() == index 
                        || prev.has_value() && prev.value() == index;
                }

            private:
                size_t m_index = 0;
                size_t m_size = 0;
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
                    blockIndex = m_list[blockIndex].next.value();
                }

                return m_list[blockIndex];
            }

            inline size_t size() const noexcept { return m_size; }
            inline block_t* list_array() const noexcept { return m_list; }
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
            void AddFreeBlock(size_t index, size_t blockSize);
            std::optional<size_t> FindAndRemoveFreeBlock(size_t numBlocks);
            bool IsBlockFree(size_t index) const noexcept;

        private:
            memory_blocks_free_list m_freeList;
        };
    }
}