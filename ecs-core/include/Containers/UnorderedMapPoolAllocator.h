#pragma once 

#include <memory>
#include <array>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <cassert>
#include <memory_resource>
#include <unordered_map>
#include "memory.h"

namespace ecs 
{
    namespace memory_pool
    {
        template<typename T, size_t PairBlockCount = 10000, size_t HashNodeBlockCount = 10000>
        class unordered_map_pool_allocator
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

            struct pairBucket_t
            {
                static constexpr size_t s_blockSize = sizeof(T);
                static constexpr size_t s_blockCount = PairBlockCount;
            };

            struct hashNodeBucket_t
            {
                static constexpr size_t s_blockSize = sizeof(std::__detail::_Hash_node<T, false>);
                static constexpr size_t s_blockCount = HashNodeBlockCount;
            };

            template<typename U>
            struct rebind 
            {
                using other = unordered_map_pool_allocator<U, PairBlockCount, HashNodeBlockCount>;
            };

            unordered_map_pool_allocator() noexcept 
                : m_upstreamResource{std::pmr::get_default_resource()} 
            {}

            unordered_map_pool_allocator(std::pmr::memory_resource* upstreamResource) noexcept 
                : m_upstreamResource{upstreamResource} 
            {}

            template<typename U>
            unordered_map_pool_allocator(const unordered_map_pool_allocator<U, PairBlockCount, HashNodeBlockCount>& other) noexcept 
                : m_upstreamResource{other.upstreamResource()} 
            {}

            template<typename U>
            unordered_map_pool_allocator& operator=(const unordered_map_pool_allocator<U, PairBlockCount, HashNodeBlockCount>& other) noexcept 
            {
                m_upstreamResource = other.upstreamResource();
                return *this;
            }

            inline std::pmr::memory_resource* upstreamResource() const noexcept 
            {
                return m_upstreamResource;
            }

            template <typename U>
            bool operator==(const unordered_map_pool_allocator<U, PairBlockCount, HashNodeBlockCount>& other) noexcept
            { 
                return m_upstreamResource == other.upstreamResource(); 
            }

            template <typename U>
            bool operator!=(const unordered_map_pool_allocator<U, PairBlockCount, HashNodeBlockCount>& other) noexcept 
            {
                return !(*this == other);
            }

            pointer allocate(size_type n, const void* hint = nullptr)
            {
                if constexpr (IsExplicitMemoryPoolDefined<pairBucket_t, hashNodeBucket_t>())
                {
                    return static_cast<pointer>(Allocate<pairBucket_t, hashNodeBucket_t>(n * sizeof(T)));
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
                if constexpr (IsExplicitMemoryPoolDefined<pairBucket_t, hashNodeBucket_t>())
                {
                    Deallocate<pairBucket_t, hashNodeBucket_t>(p, n * sizeof(T));
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
