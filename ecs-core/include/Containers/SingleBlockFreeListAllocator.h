#pragma once 

#include "memory.h"
#include <cassert>

namespace ecs
{
    template<typename T, size_t InitialCapacity = 8>
    class SingleBlockFreeListAllocator
    {
    public:
        SingleBlockFreeListAllocator()
        {
            m_capacity = InitialCapacity;

            m_data = static_cast<T*>(Malloc(sizeof(T) * m_capacity));
            m_usedCount = 0;

            m_freeIndices = static_cast<size_t*>(Malloc(sizeof(size_t) * m_capacity));
            m_freeCount = m_capacity;

            for (size_t i = 0; i < m_capacity; ++i)
            {
                m_freeIndices[i] = m_capacity - i - 1;
            }
        }

        ~SingleBlockFreeListAllocator()
        {
            Free(m_data);
            Free(m_freeIndices);
        }

        size_t AllocateBlock()
        {
            if (m_freeCount == 0)
            {
                const size_t oldCapacity = m_capacity;
                m_capacity *= 2;

                m_data = static_cast<T*>(Realloc(m_data, sizeof(T) * m_capacity));
                m_freeIndices = static_cast<size_t*>(Realloc(m_freeIndices, sizeof(size_t) * m_capacity));

                for (size_t i = m_freeCount; i < m_capacity; ++i)
                {
                    m_freeIndices[i] = m_capacity - i - 1;
                }

                m_freeCount += oldCapacity;
            }

            m_usedCount += 1;
            const size_t index = m_freeIndices[--m_freeCount];
            return index;
        }

        void FreeBlock(size_t index)
        {
            m_freeIndices[m_freeCount++] = index;
            m_usedCount -= 1;
            m_data[index].~T();
        }

        inline size_t usedCount() const noexcept { return m_usedCount; }
        inline size_t freeCount() const noexcept { return m_freeCount; }
        inline size_t capacity() const noexcept { return m_capacity; }

        T& operator[](const size_t index) const
        {
            assert(index < m_usedCount);
            return m_data[index];
        }

    private:
        T* m_data = nullptr;
        size_t m_usedCount = 0;
        size_t* m_freeIndices = nullptr;
        size_t m_freeCount = 0;
        size_t m_capacity = 0;
    };
}