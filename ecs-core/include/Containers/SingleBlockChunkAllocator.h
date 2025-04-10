#pragma once 

#include <cstdint>
#include <memory>

namespace ecs 
{
    template<typename T>
    class SingleBlockChunkAllocator
    {


    private:
        template<typename MaskType = uint32_t>
        struct chunk_t 
        {
            T data[8];
            MaskType useMask;
            
            inline bool is_allocated(size_t index) const noexcept 
            {
                return (1 << static_cast<MaskType>(index)) & useMask;
            }

            size_t allocate_block(size_t index, T value)
            {
                if (!is_allocated(index))
                {
                    useMask |= (1 << static_cast<MaskType>(index));
                    data[index] = value;
                    m_numUsed += 1;
                    return index;
                }

                throw std::runtime_error("Tried to allocate a block which is already in use.");
            }

            void free_block(size_t index)
            {
                if (is_allocated(index))
                {
                    useMask = useMask & ~(1U << index);
                    m_numUsed -= 1;
                }
            }

        private:
            size_t m_numUsed = 0;
        };


        chunk_t* m_data = nullptr;
        size_t m_numChunks = 0;
        size_t* m_freeChunks = nullptr;
        size_t m_numFreeChunks = 0;
    };
}