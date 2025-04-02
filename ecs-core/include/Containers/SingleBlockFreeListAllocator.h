#pragma once 

#include "memory.h"

namespace ecs
{
    template<typename T, size_t InitialCapacity = 8>
    class SingleBlockFreeListAllocator
    {
    public:
        SingleBlockFreeListAllocator()
        {
            m_capacity = InitialCapacity;

            m_data = Malloc(sizeof(T) * m_capacity);
            m_usedCount = 0;

            m_freeIndices = Malloc(sizeof(size_t) * m_capacity);
            m_freeIndices = m_capacity;

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

        T* AllocateBlock()
        {
            if (m_freeCount == 0)
            {
                // resize
                m_capacity *= 2;
                m_data = Realloc(m_data, sizeof(T) * m_capacity);
                m_freeIndices = Realloc(m_freeIndices, sizeof(size_t) * m_capacity);

                for (size_t i = m_freeCount; i < m_capacity; ++i)
                {
                    m_freeIndices[m_freeCount++] = m_capacity - i - 1;
                }
            }

            m_usedCount += 1;
            return static_cast<T*>(&m_data[m_freeIndices[m_freeCount--]]);
        }

        void FreeBlock(T* ptr)
        {
            void* asVoidPtr = static_cast<void*>(ptr);
            const size_t index = asVoidPtr - m_data;
            m_freeIndices[m_freeCount++] = index;
            m_usedCount -= 1;
            delete ptr;
        }

        inline size_t usedCount() const noexcept { return m_usedCount; }
        inline size_t freeCount() const noexcept { return m_freeCount; }
        inline size_t capacity() const noexcept { return m_capacity; }

    private:
        void* m_data = nullptr;
        size_t m_usedCount = 0;
        size_t* m_freeIndices = nullptr;
        size_t m_freeCount = 0;
        size_t m_capacity = 0;
    };
}