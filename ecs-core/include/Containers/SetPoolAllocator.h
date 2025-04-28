#pragma once 

#include <memory>
#include <memory_resource>
#include <cstdint>
#include <cstddef>
#include <set>
#include <iostream>
#include "Containers/DynamicBucketAllocators.h"

#ifdef __GLIBCXX__
	// GCC libstdc++ implementation
#include <bits/stl_tree.h>
#elif defined(_LIBCPP_VERSION)
    // LLVM's libc++ implementation
#include <__tree>
#elif defined(_MSC_VER)
    // Microsoft's STL implementation
#include <xtree>
#endif

namespace ecs
{
	namespace memory_pool
	{
		template<typename T, typename TElement, size_t MaxElements>
		class set_pool_memory_allocator
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
#ifdef __GLIBCXX__
            // GCC's libstdc++ implementation
            using rbnode_type = std::_Rb_tree_node<TElement>;
#elif defined(_LIBCPP_VERSION)
            // LLVM's libc++ implementation
            using rbnode_type =  __tree_node<T, void*>;
#elif defined(_MSC_VER)
            // Microsoft's STL implementation
            using rbnode_type = _Tree_node<T, void*>;
#endif

			template<typename U>
            struct rebind 
            {
                using other = set_pool_memory_allocator<U, TElement, MaxElements>;
            };

            set_pool_memory_allocator() noexcept 
                : m_upstreamResource{std::pmr::get_default_resource()} 
            {}

            set_pool_memory_allocator(std::pmr::memory_resource* upstreamResource) noexcept 
                : m_upstreamResource{upstreamResource} 
            {}

            template<typename U>
            set_pool_memory_allocator(const set_pool_memory_allocator<U, TElement, MaxElements>& other) noexcept 
                : m_upstreamResource{other.upstreamResource()} 
            {}

            template<typename U>
            set_pool_memory_allocator& operator=(const set_pool_memory_allocator<U, TElement, MaxElements>& other) noexcept 
            {
                m_upstreamResource = other.upstreamResource();
                return *this;
            }

            inline std::pmr::memory_resource* upstreamResource() const noexcept 
            {
                return m_upstreamResource;
            }

            template <typename U>
            bool operator==(const set_pool_memory_allocator<U, TElement, MaxElements>& other) noexcept
            { 
                return m_upstreamResource == other.upstreamResource(); 
            }

            template <typename U>
            bool operator!=(const set_pool_memory_allocator<U, TElement, MaxElements>& other) noexcept 
            {
                return !(*this == other);
            }

			struct rbNodeBucket_t
            {
                static constexpr size_t s_blockSize = sizeof(rbnode_type);
                static constexpr size_t s_blockCount = MaxElements;
            };

			pointer allocate(size_type n, const void* hint = nullptr)
            {
                if constexpr (IsDynamicMemoryPoolDefined<rbNodeBucket_t>())
                {
                    return static_cast<pointer>(AllocateFromDynamicBucket<rbNodeBucket_t>(n 
                        * sizeof(T), 0));
                }
                else if (m_upstreamResource != nullptr)
                {
                    return static_cast<pointer>(m_upstreamResource->allocate(n * sizeof(T), 
                        alignof(T)));
                }
                else
                {
                    throw std::bad_alloc();
                }
            }

            void deallocate(pointer p, size_type n)
            {
                if constexpr (IsDynamicMemoryPoolDefined<rbNodeBucket_t>())
                {
                    DeallocateFromDynamicBucket<rbNodeBucket_t>(p, n * sizeof(T));
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