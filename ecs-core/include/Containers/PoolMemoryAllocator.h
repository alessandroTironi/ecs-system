#pragma once 

#include <memory>
#include <array>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <cassert>
#include <memory_resource>
#include "memory.h"

namespace ecs
{
    namespace memory_pool
    {
        template<typename T = uint8_t, size_t Id = 1>
        class pool_memory_allocator
        {
        public:
            using value_type = T;
            using pointer = T*;
            using const_pointer = const T*;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using propagate_on_container_copy_assignment = std::false_type;
            using propagate_on_container_move_assignment = std::true_type;
            using propagate_on_container_swap = std::false_type;
            using is_always_equal = std::false_type;  

            template<typename U>
            struct rebind 
            {
                using other = pool_memory_allocator<U, Id>;
            };

            pool_memory_allocator() noexcept 
                : m_upstreamResource{std::pmr::get_default_resource()} 
            {}

            pool_memory_allocator(std::pmr::memory_resource* upstreamResource) noexcept 
                : m_upstreamResource{upstreamResource} 
            {}

            template<typename U>
            pool_memory_allocator(const pool_memory_allocator<U, Id>& other) noexcept 
                : m_upstreamResource{other.upstreamResource()} 
            {}

            template<typename U>
            pool_memory_allocator& operator=(const pool_memory_allocator<U, Id>& other) noexcept 
            {
                m_upstreamResource = other.upstreamResource();
                return *this;
            }

            inline std::pmr::memory_resource* upstreamResource() const noexcept 
            {
                return m_upstreamResource;
            }

            template <typename U>
            bool operator==(const pool_memory_allocator<U, Id>& other) noexcept
            { 
                return m_upstreamResource == other.upstreamResource(); 
            }

            template <typename U>
            bool operator!=(const pool_memory_allocator<U, Id>& other) noexcept 
            {
                return !(*this == other);
            }

            static bool initializeMemoryPool() noexcept 
            {
                return InitializeMemoryPool<Id>();
            }

            pointer allocate(size_type n, const void* hint = nullptr)
            {
                if constexpr (IsMemoryPoolDefined<Id>())
                {
                    return static_cast<pointer>(Allocate<Id>(n * sizeof(T)));
                }
                else if (m_upstreamResource != nullptr)
                {
                    return static_cast<pointer>(m_upstreamResource->allocate(n * sizeof(T), alignof(T)));
                }
                else
                {
                    throw std::bad_alloc();
                }
            }

            void deallocate(pointer p, size_type n) 
            {
                if constexpr (IsMemoryPoolDefined<Id>())
                {
                    Deallocate<Id>(p, n * sizeof(T));
                }
                else if (m_upstreamResource != nullptr)
                {
                    m_upstreamResource->deallocate(p, n * sizeof(T), alignof(T));
                }
                else 
                {
                    assert(false);
                }
            }
            
        private:
            std::pmr::memory_resource* m_upstreamResource{nullptr};
        };
    }
}