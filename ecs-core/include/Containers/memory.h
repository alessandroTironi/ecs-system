#pragma once 

#include <cstdint>
#include <cstddef>
#include <tuple>
#include <algorithm>

namespace ecs
{
    void* Malloc(size_t size);

    void Free(void* ptr);

    void* Realloc(void* ptr, size_t size);

    void* Calloc(size_t count, size_t size);

    namespace mem 
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

        template<size_t id>
        static constexpr size_t bucketCount = std::tuple_size<bucket_descriptors_t<id>>::value;

        template<size_t id>
        using poolType = std::array<bucket_t, bucketCount<id>>;

        template<size_t id, size_t idx>
        struct get_size : std::integral_constant<size_t, 
            std::tuple_element_t<idx, bucket_descriptors_t<id>>::s_blockSize> 
        {};

        template<size_t id, size_t idx>
        struct get_count : std::integral_constant<size_t, 
            std::tuple_element<idx, bucket_descriptors_t<id>>::s_blockCount>
        {};

        template<size_t id, size_t... Idx>
        auto& get_instance(std::index_sequence<Idx...>) noexcept 
        {
            static poolType<id> instance{{{get_size<id, Idx>::value, get_count<id, Idx>::value} ...}};
            return instance;
        }

        template<size_t id>
        auto& get_instance() noexcept 
        {
            return get_instance<id>(std::make_index_sequence<bucketCount<id>>());
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

        template<size_t id>
        [[nodiscard]] void* allocate(size_t bytes)
        {
            poolType<id>& pool = get_instance<id>();
            std::array<allocation_info_t, bucketCount<id>> deltas;
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
                    return ptr;
                }
            }

            throw std::bad_alloc();
        }

        template<size_t Id>
        void deallocate(void* ptr, size_t bytes) noexcept
        {
            poolType<Id>& pool = get_instance<Id>();

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

        /**
         * @brief Initializes a memory pool with the provided ID.
         * 
         * @tparam Id of the memory pool.
         * @return true if the pool was properly found or initialized.
         */
        template<size_t Id>
        bool InitializeMemoryPool() noexcept 
        {
            (void) get_instance<Id>();
            return IsMemoryPoolDefined<Id>();
        }
    }
}