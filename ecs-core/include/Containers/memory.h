#pragma once 

#include <cstdint>
#include <cstddef>
#include <tuple>
#include <algorithm>
#include <iostream>

namespace ecs
{
    void* Malloc(size_t size);

    void Free(void* ptr);

    void* Realloc(void* ptr, size_t size);

    void* Calloc(size_t count, size_t size);

    namespace memory_pool
    {
        /**
         * @brief A bucket of memory blocks.
         */
        struct bucket_t
        {
        public:
            const size_t blockSize;
            const size_t blockCount;

            bucket_t(size_t inBlockSize, size_t inBlockCount);
            ~bucket_t();

            bool belongs_to_this(void* ptr) const noexcept;

            [[nodiscard]] void* allocate(size_t bytes) noexcept;
            void deallocate(void* ptr, size_t bytes) noexcept;

        private:
            /** 
             * Finds n free contiguous blocks in the ledger. 
             * @param n The amount of needed contiguous blocks.
             * @return The first block's index, or blockCount on failure.
             */
            size_t find_contiguous_blocks(size_t n) const noexcept;

            /**
             * @brief Marks n blocks in the ledger as "in use" starting from index.
             * 
             * @param index Index of the first block.
             * @param n Amount of blocks to be marked as "in-use".
             */
            void set_blocks_in_use(size_t index, size_t n) noexcept;

            /**
             * @brief Marks n blocks in the ledger as "free" starting from index.
             * 
             * @param index Index of the first block.
             * @param n Amount of blocks to be marked as free.
             */
            void set_blocks_free(size_t index, size_t n) noexcept;

            /**
             * @brief Tells wether a memory block is currently in use or not.
             * 
             * @param index of the memory block to test
             * @return true if the memory block is in use 
             */
            bool is_block_in_use(size_t index) const noexcept;

            /** Contains allocated data. */
            std::byte* m_data{nullptr};

            /** bitmask to mark each block as free or in-use. */
            std::byte* m_ledger{nullptr};
        };

        // The default implementation defines a pool with no buckets.
        template<size_t id>
        struct bucket_descriptors
        {
            using type = std::tuple<>;
        };

        template<typename... TBuckets>
        struct explicit_bucket_descriptors
        {
            using type = std::tuple<TBuckets...>;
        };

        // Specialization of bucket instances
        struct bucket_cfg16
        {
            static constexpr size_t s_blockSize = 16;
            static constexpr size_t s_blockCount = 10000;
        };

        struct bucket_cfg32
        {
            static constexpr size_t s_blockSize = 32;
            static constexpr size_t s_blockCount = 10000;
        };

        struct bucket_cfg1024
        {
            static constexpr size_t s_blockSize = 1024;
            static constexpr size_t s_blockCount = 50000;
        };

        template<>
        struct bucket_descriptors<1>
        {
            using type = std::tuple<bucket_cfg16, bucket_cfg32, bucket_cfg1024>;
        };

        template<size_t id>
        using bucket_descriptors_t = typename bucket_descriptors<id>::type;

        template<typename... TBuckets>
        using explicit_bucket_descriptors_t = typename explicit_bucket_descriptors<TBuckets...>::type;

        template<size_t id>
        static constexpr size_t bucketCount = std::tuple_size<bucket_descriptors_t<id>>::value;

        template<typename... TBuckets>
        static constexpr size_t explicitBucketCount = std::tuple_size<explicit_bucket_descriptors_t<TBuckets...>>::value;

        template<size_t id>
        using MemoryPool = std::array<bucket_t, bucketCount<id>>;

        template<typename... TBuckets>
        using ExplicitMemoryPool = std::array<bucket_t, explicitBucketCount<TBuckets...>>;

        template<size_t id, size_t idx>
        struct get_size : std::integral_constant<size_t, 
            std::tuple_element_t<idx, bucket_descriptors_t<id>>::s_blockSize> 
        {};

        template<typename TBucket>
        struct get_size_explicit : std::integral_constant<size_t, TBucket::s_blockSize>
        {};

        template<size_t id, size_t idx>
        struct get_count : std::integral_constant<size_t, 
            std::tuple_element_t<idx, bucket_descriptors_t<id>>::s_blockCount>
        {};

        template<typename TBucket>
        struct get_count_explicit : std::integral_constant<size_t, TBucket::s_blockCount>
        {};

        template<size_t id, size_t... Idx>
        auto& GetMemoryPool(std::index_sequence<Idx...>) noexcept 
        {
            static MemoryPool<id> instance{{{get_size<id, Idx>::value, get_count<id, Idx>::value} ...}};
            return instance;
        }

        template<size_t id>
        auto& GetMemoryPool() noexcept 
        {
            return GetMemoryPool<id>(std::make_index_sequence<bucketCount<id>>());
        }

        template<typename... TBuckets>
        auto& GetExplicitMemoryPool() noexcept 
        {
            static ExplicitMemoryPool<TBuckets...> instance{{{get_size_explicit<TBuckets>::value, get_count_explicit<TBuckets>::value} ...}};
            return instance;
        }

        /**
         * @brief Holds information about a specific allocation request.
         */
        struct allocation_info_t
        {
            size_t index{0};            // which bucket?
            size_t blockCount{0};       // how many blocks would the allocation take from the bucket?
            size_t waste{0};            // how much memory would be wasted?

            bool operator<(const allocation_info_t& other) const noexcept 
            {
                return (waste == other.waste)? blockCount < other.blockCount : waste < other.waste;
            }
        };

        template<size_t Id>
        [[nodiscard]] void* Allocate(size_t bytes)
        {
            MemoryPool<Id>& pool = GetMemoryPool<Id>();
            std::array<allocation_info_t, bucketCount<Id>> deltas;
            size_t index = 0;
            const size_t id = Id;

            for (const bucket_t& bucket : pool)
            {
                deltas[index].index = index;
                if (bucket.blockSize >= bytes)
                {
                    deltas[index].waste = bucket.blockSize - bytes;
                    deltas[index].blockCount = 1;
                }
                else
                {
                    const size_t n = 1 + ((bytes - 1) / bucket.blockSize);
                    const size_t storageRequired = n * bucket.blockSize;
                    deltas[index].waste = storageRequired - bytes;
                    deltas[index].blockCount = n;
                }

                ++index;
            }

            std::sort(deltas.begin(), deltas.end());

            for (const allocation_info_t& d : deltas)
            {
                if (void* ptr = pool[d.index].allocate(bytes); ptr != nullptr)
                {
                    return ptr;
                }
            }

            throw std::bad_alloc();
        }

        template<typename... TBuckets>
        [[nodiscard]] void* Allocate(size_t bytes)
        {
            ExplicitMemoryPool<TBuckets...>& pool = GetExplicitMemoryPool<TBuckets...>();
            std::array<allocation_info_t, explicitBucketCount<TBuckets...>> deltas;
            size_t index = 0;

            for (const bucket_t& bucket : pool)
            {
                deltas[index].index = index;
                if (bucket.blockSize >= bytes)
                {
                    deltas[index].waste = bucket.blockSize - bytes;
                    deltas[index].blockCount = 1;
                }
                else
                {
                    const size_t n = 1 + ((bytes - 1) / bucket.blockSize);
                    const size_t storageRequired = n * bucket.blockSize;
                    deltas[index].waste = storageRequired - bytes;
                    deltas[index].blockCount = n;
                }

                ++index;
            }

            std::sort(deltas.begin(), deltas.end());

            for (const allocation_info_t& d : deltas)
            {
                if (void* ptr = pool[d.index].allocate(bytes); ptr != nullptr)
                {
                    std::cout << "Picked allocator " << d.index << std::endl;
                    return ptr;
                }
            }

            throw std::bad_alloc();
        }

        template<typename... TBuckets>
        [[nodiscard]] void* AllocateFromSpecificBucket(size_t bytes, size_t index)
        {
            ExplicitMemoryPool<TBuckets...>& pool = GetExplicitMemoryPool<TBuckets...>();
            if (index >= explicitBucketCount<TBuckets...>)
            {
                throw std::bad_alloc();
            }

            bucket_t& bucket = pool[index];
            if (void* ptr = bucket.allocate(bytes); ptr != nullptr)
            {
                return ptr;
            }

            throw std::bad_alloc();
        }
        

        template<size_t Id>
        void Deallocate(void* ptr, size_t bytes) noexcept
        {
            MemoryPool<Id>& pool = GetMemoryPool<Id>();

            for (bucket_t& bucket : pool)
            {
                if (bucket.belongs_to_this(ptr))
                {
                    bucket.deallocate(ptr, bytes);
                    break;
                }
            }
        }

        template<typename... TBuckets>
        void Deallocate(void* ptr, size_t bytes) noexcept 
        {
            ExplicitMemoryPool<TBuckets...>& pool = GetExplicitMemoryPool<TBuckets...>();
            for (bucket_t& bucket : pool)
            {
                if (bucket.belongs_to_this(ptr))
                {
                    bucket.deallocate(ptr, bytes);
                    break;
                }
            }
        }


        /**
         * @brief Tells if a memory pool with the provided ID is actually defined.
         * 
         * @tparam Id of the memory pool
         * @return true if a memory pool with such ID is defined.
         */
        template<size_t Id>
        constexpr bool IsMemoryPoolDefined() noexcept 
        {
            return bucketCount<Id> != 0;
        }

        template<typename... TBuckets>
        constexpr bool IsExplicitMemoryPoolDefined() noexcept 
        {
            return explicitBucketCount<TBuckets...> != 0;
        }

        /**
         * @brief Initializes a memory pool with the provided ID.
         * 
         * @tparam Id of the memory pool.
         * @return true if the pool was properly found or initialized.
         */
        template<size_t Id>
        bool InitializeMemoryPool() noexcept 
        {
            (void) GetMemoryPool<Id>();
            return IsMemoryPoolDefined<Id>();
        }

        template<typename... TBuckets>
        bool InitializeExplicitMemoryPool() noexcept 
        {
            (void) GetExplicitMemoryPool<TBuckets...>();
            return IsExplicitMemoryPoolDefined<TBuckets...>();
        }
    }
}