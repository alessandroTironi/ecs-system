#pragma once 

#include <cstdint>
#include <memory>
#include <limits>
#include <string>
#include <format>
#include "memory.h"

namespace ecs 
{
    template<typename T>
    class SingleBlockChunkAllocator
    {
    public:
        SingleBlockChunkAllocator()
        {
            m_capacity = 8;
            m_numChunks = (m_capacity >> 5) + 1;
            m_capacity = m_numChunks * 32;
            m_data = static_cast<chunk_t*>(Malloc(sizeof(chunk_t) * m_numChunks));
            for (size_t chunkIndex = 0; chunkIndex < m_numChunks; ++chunkIndex)
            {
                m_data[chunkIndex] = chunk_t();
            }

            m_numFreeIndices = m_numChunks * 32;
            m_freeIndices = static_cast<size_t*>(Malloc(sizeof(size_t) * m_numFreeIndices));
            for (size_t i = 0; i < m_numFreeIndices; ++i)
            {
                m_freeIndices[i] = m_numFreeIndices - i - 1;
            }
        }

        ~SingleBlockChunkAllocator()
        {
            Free(m_data);
            Free(m_freeIndices);
        }

        size_t AllocateBlock()
        {
            if (m_numFreeIndices == 0)
            {
                // need to allocate new chunks
                m_capacity *= 2;
                const size_t previousNumChunks = m_numChunks;
                m_numChunks = (m_capacity >> 5) + 1;
                m_data = static_cast<chunk_t*>(Realloc(m_data, sizeof(chunk_t) * m_numChunks));

                for (size_t chunkIndex = previousNumChunks; chunkIndex < m_numChunks; ++chunkIndex)
                {
                    m_data[chunkIndex] = chunk_t();
                }

                const size_t previousNumFreeIndices = m_numFreeIndices;
                m_numFreeIndices = (m_numChunks - previousNumChunks) * 32;
                m_freeIndices = static_cast<size_t*>(Realloc(m_freeIndices, sizeof(size_t) * m_numFreeIndices));

                for (size_t i = 0; i < m_numFreeIndices; ++i)
                {
                    m_freeIndices[i] = previousNumChunks * 32 + i;
                }
            }

            const size_t nextFreeIndex = m_freeIndices[m_numFreeIndices - 1];
            m_numFreeIndices -= 1;

            const size_t chunkIndex = nextFreeIndex >> 5;
            chunk_t& freeChunk = m_data[chunkIndex];
            const size_t chunkLocalIndex = nextFreeIndex - (chunkIndex * 32);

            freeChunk.allocate_block(chunkLocalIndex, T());
            return nextFreeIndex;
        }

        void FreeBlock(const size_t index)
        {
            const size_t chunkIndex = index >> 5;
            chunk_t& freeChunk = m_data[chunkIndex];
            const size_t blockIndex = index - (chunkIndex * 32);

            freeChunk.free_block(blockIndex);
            m_freeIndices[m_numFreeIndices++] = index;
        }

        T& operator[](const size_t index) const 
        {
            const size_t chunkIndex = index >> 5;
            chunk_t& freeChunk = m_data[chunkIndex];
            const size_t blockIndex = index - (chunkIndex * 32);

            return m_data[chunkIndex].data[blockIndex];
        }

        inline size_t usedCount() const noexcept { return m_numChunks * 32 - m_numFreeIndices; }
        inline size_t freeCount() const noexcept { return m_numFreeIndices; }
        inline size_t capacity() const noexcept { return m_capacity; }

    private:
        struct chunk_t 
        {
            static constexpr size_t NumBits = sizeof(uint32_t) * 8;

            T data[NumBits];
            uint32_t useMask;

            chunk_t()
            {
                m_numUsed = 0;
                useMask = 0U;
            }
            
            inline bool is_block_allocated(size_t index) const noexcept 
            {
                return (1 << static_cast<uint32_t>(index)) & useMask;
            }

            size_t allocate_block(size_t index, T value)
            {
                if (!is_block_allocated(index))
                {
                    useMask |= (1 << static_cast<uint32_t>(index));
                    data[index] = value;
                    m_numUsed += 1;
                    return index;
                }

                std::string error = std::format("Tried to allocate a block which was already in use: {}", 
                    index);
                throw std::runtime_error(error);
            }

            void free_block(size_t index)
            {
                if (is_block_allocated(index))
                {
                    useMask = useMask & ~(1U << index);
                    m_numUsed -= 1;
                }
            }

            bool is_fully_allocated() const 
            {
                return useMask == 0xffffffff;
            }

            inline size_t capacity() const { return NumBits; }

        private:
            size_t m_numUsed = 0;
        };


        chunk_t* m_data = nullptr;
        size_t m_numChunks = 0;
        size_t m_capacity;

        size_t* m_freeIndices = nullptr;
        size_t m_numFreeIndices = 0;
    };
}