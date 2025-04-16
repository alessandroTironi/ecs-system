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
            s_instanceCount += 1;
            if (s_instanceCount == 1)
            {
                s_capacity = 8;
                s_numChunks = (s_capacity >> 5) + 1;
                s_capacity = s_numChunks * 32;
                s_data = static_cast<chunk_t*>(Malloc(sizeof(chunk_t) * s_numChunks));
                for (size_t chunkIndex = 0; chunkIndex < s_numChunks; ++chunkIndex)
                {
                    s_data[chunkIndex] = chunk_t();
                }

                s_numFreeIndices = s_numChunks * 32;
                s_freeIndices = static_cast<size_t*>(Malloc(sizeof(size_t) * s_numFreeIndices));
                for (size_t i = 0; i < s_numFreeIndices; ++i)
                {
                    s_freeIndices[i] = s_numFreeIndices - i - 1;
                }
            }
        }

        ~SingleBlockChunkAllocator()
        {
            s_instanceCount -= 1;
            if (s_instanceCount == 0)
            {
                Free(s_data);
                Free(s_freeIndices);
            }
        }

        size_t AllocateBlock()
        {
            if (s_numFreeIndices == 0)
            {
                // need to allocate new chunks
                s_capacity *= 2;
                const size_t previousNumChunks = s_numChunks;
                s_numChunks = (s_capacity >> 5) + 1;
                s_data = static_cast<chunk_t*>(Realloc(s_data, sizeof(chunk_t) * s_numChunks));

                for (size_t chunkIndex = previousNumChunks; chunkIndex < s_numChunks; ++chunkIndex)
                {
                    s_data[chunkIndex] = chunk_t();
                }

                const size_t previousNumFreeIndices = s_numFreeIndices;
                s_numFreeIndices = (s_numChunks - previousNumChunks) * 32;
                s_freeIndices = static_cast<size_t*>(Realloc(s_freeIndices, sizeof(size_t) * s_numFreeIndices));

                for (size_t i = 0; i < s_numFreeIndices; ++i)
                {
                    s_freeIndices[i] = previousNumChunks * 32 + i;
                }
            }

            const size_t nextFreeIndex = s_freeIndices[s_numFreeIndices - 1];
            s_numFreeIndices -= 1;

            const size_t chunkIndex = nextFreeIndex >> 5;
            chunk_t& freeChunk = s_data[chunkIndex];
            const size_t chunkLocalIndex = nextFreeIndex - (chunkIndex * 32);

            freeChunk.allocate_block(chunkLocalIndex, T());
            return nextFreeIndex;
        }

        void FreeBlock(const size_t index)
        {
            const size_t chunkIndex = index >> 5;
            chunk_t& freeChunk = s_data[chunkIndex];
            const size_t blockIndex = index - (chunkIndex * 32);

            freeChunk.free_block(blockIndex);
            s_freeIndices[s_numFreeIndices++] = index;
        }

        T& operator[](const size_t index) const 
        {
            const size_t chunkIndex = index >> 5;
            chunk_t& freeChunk = s_data[chunkIndex];
            const size_t blockIndex = index - (chunkIndex * 32);

            return s_data[chunkIndex].data[blockIndex];
        }

        inline size_t usedCount() const noexcept { return s_numChunks * 32 - s_numFreeIndices; }
        inline size_t freeCount() const noexcept { return s_numFreeIndices; }
        inline size_t capacity() const noexcept { return s_capacity; }

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


        inline static chunk_t* s_data = nullptr;
        inline static size_t s_numChunks = 0;
        inline static size_t s_capacity;

        inline static size_t* s_freeIndices = nullptr;
        inline static size_t s_numFreeIndices = 0;

        inline static size_t s_instanceCount = 0;
    };
}