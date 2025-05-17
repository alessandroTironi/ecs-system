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
            if (m_bufferCount.load(std::memory_order_relaxed) >= m_capacity)
            {
                // buffer is full
                return false;
            }

            size_t index = m_producerIndex.load(std::memory_order_relaxed);
            m_data[index] = item; 
            m_producerIndex.store((index + 1) % m_capacity, std::memory_order_relaxed);
            m_bufferCount.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        // Overload for move semantics
        inline bool produce_item(T&& item) noexcept 
        {
            if (m_bufferCount.load(std::memory_order_relaxed) >= m_capacity)
            {
                // buffer is full
                return false;
            }

            size_t index = m_producerIndex.load(std::memory_order_relaxed);
            m_data[index] = std::move(item); 
            m_producerIndex.store((index + 1) % m_capacity, std::memory_order_relaxed);
            m_bufferCount.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        inline bool consume_item(T& outItem) noexcept 
        {
            if (m_bufferCount.load(std::memory_order_relaxed) == 0)
            {
                // buffer empty
                return false;
            }

            size_t index = m_consumerIndex.load(std::memory_order_relaxed);
            outItem = m_data[index]; 
            m_consumerIndex.store((index + 1) % m_capacity, std::memory_order_relaxed);
            m_bufferCount.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }

        inline void resize_buffer(size_t newCapacity)
        {
            if (newCapacity <= m_capacity) 
			{
                return;  
            }

            // Allocate new memory
            T* newData = static_cast<T*>(Malloc(sizeof(T) * newCapacity));
            
            // Construct new elements
            for (size_t i = 0; i < newCapacity; ++i) 
			{
                if (i < m_capacity) 
				{
                    new (&newData[i]) T(std::move(m_data[i]));
                    m_data[i].~T();
                } 
				else 
				{
                    new (&newData[i]) T();
                }
            }
            
            // Free old memory
            Free(m_data);
            
            m_data = newData;
            m_capacity = newCapacity;
            
            // Reset indices if they're beyond new capacity
            size_t producerIdx = m_producerIndex.load(std::memory_order_relaxed);
            size_t consumerIdx = m_consumerIndex.load(std::memory_order_relaxed);
            
            if (producerIdx >= newCapacity) 
			{
                m_producerIndex.store(producerIdx % newCapacity, std::memory_order_relaxed);
            }
            
            if (consumerIdx >= newCapacity) 
			{
                m_consumerIndex.store(consumerIdx % newCapacity, std::memory_order_relaxed);
            }
        }

    private:
        T* m_data = nullptr;
        std::atomic<size_t> m_bufferCount{0};
        std::atomic<size_t> m_producerIndex{0};
        std::atomic<size_t> m_consumerIndex{0};
        size_t m_capacity = 8;
    };
}