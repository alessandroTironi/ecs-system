#pragma once 

#include <atomic>
#include <memory>
#include "memory.h"

namespace ecs 
{
    template<typename T>
    struct ring_buffer
    {
    public:
        ring_buffer()
        {
            m_capacity = 8;
            m_data = static_cast<T*>(Malloc(sizeof(T) * m_capacity));
            
            for (size_t i = 0; i < m_capacity; ++i) 
            {
                new (&m_data[i]) T();
            }
        }

        ~ring_buffer()
        {
            for (size_t i = 0; i < m_capacity; ++i) 
            {
                m_data[i].~T();
            }

            Free(m_data);
        }

        inline bool produce_item(const T& item) noexcept 
        {
            if (m_bufferCount.load(std::memory_order_acquire) >= m_capacity)
            {
                // buffer is full
                return false;
            }

            size_t index = m_producerIndex.load(std::memory_order_relaxed);
            m_data[index] = item; 
            
            // Use release ordering to ensure data is visible before index update
            m_producerIndex.store((index + 1) % m_capacity, std::memory_order_release);
            m_bufferCount.fetch_add(1, std::memory_order_release);
            return true;
        }

        // Overload for move semantics
        inline bool produce_item(T&& item) noexcept 
        {
            if (m_bufferCount.load(std::memory_order_acquire) >= m_capacity)
            {
                // buffer is full
                return false;
            }

            const size_t index = m_producerIndex.load(std::memory_order_relaxed);
            m_data[index] = std::move(item); 
            
            // Use release ordering to ensure data is visible before index update
            m_producerIndex.store((index + 1) % m_capacity, std::memory_order_release);
            m_bufferCount.fetch_add(1, std::memory_order_release);
            return true;
        }

        template<typename... Args>
        inline bool emplace_item(Args&&... args) noexcept
        {
            if (m_bufferCount.load(std::memory_order_acquire) >= m_capacity)
            {
                return false;
            }

            const size_t index = m_producerIndex.load(std::memory_order_relaxed);
            m_data[index].~T();
            new(&m_data[index]) T(std::forward<Args>(args)...);

            // Use release ordering to ensure data is visible before index update
            m_producerIndex.store((index + 1) % m_capacity, std::memory_order_release);
            m_bufferCount.fetch_add(1, std::memory_order_release);
            return true;
        }

        inline bool consume_item(T& outItem) noexcept 
        {
            if (m_bufferCount.load(std::memory_order_acquire) == 0)
            {
                // buffer empty
                return false;
            }

            const size_t index = m_consumerIndex.load(std::memory_order_relaxed);
            outItem = m_data[index]; 
            
            // Use release ordering to ensure index update is visible to producers
            m_consumerIndex.store((index + 1) % m_capacity, std::memory_order_release);
            m_bufferCount.fetch_sub(1, std::memory_order_release);
            return true;
        }

        inline void resize_buffer(size_t newCapacity)
        {
            if (newCapacity <= m_capacity) 
            {
                return;  
            }

            const size_t count = m_bufferCount.load(std::memory_order_acquire);
            const size_t producerIdx = m_producerIndex.load(std::memory_order_acquire);
            const size_t consumerIdx = m_consumerIndex.load(std::memory_order_acquire);
            
            // Allocate new memory
            T* newData = static_cast<T*>(Malloc(sizeof(T) * newCapacity));
            for (size_t i = 0; i < m_capacity; ++i)
            {
                new (&newData[i]) T(std::move(m_data[i]));
                m_data[i].~T();
            }

            for (size_t i = m_capacity; i < newCapacity; ++i)
            {
                new (&newData[i]) T();
            }
            
            // Free old memory
            Free(m_data);
            
            m_data = newData;
            m_capacity = newCapacity;
            
            // Reset indices if they're beyond new capacity
            if (producerIdx >= newCapacity) 
            {
                m_producerIndex.store(producerIdx % newCapacity, std::memory_order_release);
            }
            
            if (consumerIdx >= newCapacity) 
            {
                m_consumerIndex.store(consumerIdx % newCapacity, std::memory_order_release);
            }
        }

        inline size_t available_space() const noexcept
        {
            return m_capacity - m_bufferCount.load(std::memory_order_acquire);
        }
        
        inline size_t size() const noexcept
        {
            return m_bufferCount.load(std::memory_order_acquire);
        }
        
        inline size_t capacity() const noexcept
        {
            return m_capacity;
        }

    private:
        T* m_data = nullptr;
        std::atomic<size_t> m_bufferCount{0};
        std::atomic<size_t> m_producerIndex{0};
        std::atomic<size_t> m_consumerIndex{0};
        size_t m_capacity = 8;
    };
}