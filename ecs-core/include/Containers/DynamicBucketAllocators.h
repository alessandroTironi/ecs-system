#pragma once

#include <tuple>
#include <vector>
#include "memory.h"

namespace ecs
{
	namespace memory_pool
	{
		struct dynamic_bucket_t
        {
        public:
            const size_t blockSize;
            const size_t blockCount;

            dynamic_bucket_t(size_t inBlockSize, size_t inBlockCount);
            ~dynamic_bucket_t();

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

			struct bucket_instance_t
			{
				bucket_instance_t() {}
				bucket_instance_t(std::byte* inData, std::byte* inLedger)
					: data{inData}, ledger{inLedger} {}

				std::byte* data{nullptr};
				std::byte* ledger{nullptr};
			};

            /**
             * @brief Allocates a new instance of this bucket
             */
			void allocate_bucket_instance();

            /**
             * @brief Calculates the index of the bucket instance that contains a specific block.
             * 
             * @param blockIndex index of the block for which we are searching the bucket instance.
             * @return index of the bucket instance.
             */
			size_t calculate_bucket_instance_index(const size_t blockIndex) const;

            /**
             * @brief Finds the index of the bucket instance that contains a specific pointer.
             * 
             * @param ptr the pointer for which we are searching the bucket instance. 
             * @return index of the bucket instance, or m_data.size() if not found.
             */
			size_t find_bucket_instance_index(void* ptr) const;

            /** Contains allocated data. */
            std::vector<bucket_instance_t> m_data;
        };

		/**
		 * A unique set of dynamic buckets is defined by a tuple of descriptors.
		 */
		template<typename... TBuckets>
        struct dynamic_bucket_descriptors
        {
            using type = std::tuple<TBuckets...>;
        };


		// Quick access to the std::tuple::type member
		template<typename... TBuckets>
        using dynamic_bucket_descriptors_t = typename dynamic_bucket_descriptors<TBuckets...>::type;

		/**
		 * Calculates the amount of dynamic buckets in a descriptor array at compile time.
		 */
		template<typename... TBuckets>
        static constexpr size_t DynamicBucketCount = 
			std::tuple_size<dynamic_bucket_descriptors_t<TBuckets...>>::value;

		/**
		 * A dynamic memory pool is a memory pool that allows for growth if the capacity is exceeded.
		 */
		template<typename... TBuckets>
		using DynamicMemoryPool = std::array<dynamic_bucket_t, DynamicBucketCount<TBuckets...>>;

		template<typename TBucket>
        struct GetDynamicBucketBlockSize : std::integral_constant<size_t, TBucket::s_blockSize>
        {};

		template<typename TBucket>
        struct GetDynamicBucketBlockCount : std::integral_constant<size_t, TBucket::s_blockCount>
        {};

		/**
		 * Retrieves the unique instance of the dynamic memory pool defined by the provided bucket
		 * descriptors.
		 */
		template<typename... TBuckets>
        DynamicMemoryPool<TBuckets...>& GetDynamicMemoryPool() noexcept 
        {
            static DynamicMemoryPool<TBuckets...> instance
			{
				{
					{
						GetDynamicBucketBlockSize<TBuckets>::value, GetDynamicBucketBlockCount<TBuckets>::value
					} ...
				}
			};

            return instance;
        }

		template<typename... TBuckets>
        [[nodiscard]] void* AllocateFromDynamicBucket(size_t bytes, size_t index)
        {
            DynamicMemoryPool<TBuckets...>& pool = GetDynamicMemoryPool<TBuckets...>();
            if (index >= DynamicBucketCount<TBuckets...>)
            {
                throw std::bad_alloc();
            }

            dynamic_bucket_t& bucket = pool[index];
            if (void* ptr = bucket.allocate(bytes); ptr != nullptr)
            {
                return ptr;
            }

            throw std::bad_alloc();
        }

		template<typename... TBuckets>
        void DeallocateFromDynamicBucket(void* ptr, size_t bytes) noexcept 
        {
            DynamicMemoryPool<TBuckets...>& pool = GetDynamicMemoryPool<TBuckets...>();
            for (dynamic_bucket_t& bucket : pool)
            {
                if (bucket.belongs_to_this(ptr))
                {
                    bucket.deallocate(ptr, bytes);
                    break;
                }
            }
        }

        template<typename... TBuckets>
        constexpr bool IsDynamicMemoryPoolDefined() noexcept 
        {
            return DynamicBucketCount<TBuckets...> != 0;
        }

        template<typename... TBuckets>
        bool InitializeDynamicMemoryPool() noexcept 
        {
            (void) GetDynamicMemoryPool<TBuckets...>();
            return IsDynamicMemoryPoolDefined<TBuckets...>();
        }
	}
}