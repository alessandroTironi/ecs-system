#pragma once

#include <tuple>
#include <vector>
#include <optional>
#include "memory.h"
#include "MemoryBlocksFreeList.h"

namespace ecs
{
	namespace memory_pool
	{
		struct bucket_container_t
        {
        public:
            const size_t blockSize;
            const size_t blockCount;

            bucket_container_t(size_t inBlockSize, size_t inBlockCount);
            ~bucket_container_t();

            bool belongs_to_this(void* ptr) const noexcept;

            [[nodiscard]] void* allocate(size_t bytes) noexcept;
            void deallocate(void* ptr, size_t bytes) noexcept;

        private:
            /** 
             * Finds n free contiguous blocks in the ledger. 
             * @param n The amount of needed contiguous blocks.
             * @return The first block's index, or blockCount on failure.
             */
            std::optional<size_t> find_contiguous_blocks(size_t n) const noexcept;

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

			struct dynamic_bucket_t
			{
				dynamic_bucket_t() {}
				dynamic_bucket_t(std::byte* inData, std::byte* inLedger, bool isMemoryBlockStart)
					: data{inData}, ledger{inLedger}, m_isMemoryBlockStart(isMemoryBlockStart) {}

				std::byte* data{nullptr};
				std::byte* ledger{nullptr};
                
                inline bool is_memory_block_start() const noexcept { return m_isMemoryBlockStart; }
            private:
                bool m_isMemoryBlockStart = true;
			};

            /**
             * @brief Allocates a new instance of this bucket
             */
			void allocate_contiguous_bucket_instances(size_t numInstances);

            bool are_blocks_contiguous(size_t index1, size_t index2) const;

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
            std::vector<dynamic_bucket_t> m_data;

            FreeMemoryTracker m_memoryTracker;

            /** Contains pointers to portions of memory exceeding the default block count. */
            std::vector<void*> m_fallbackAllocations;
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
		using DynamicMemoryPool = std::array<bucket_container_t, DynamicBucketCount<TBuckets...>>;

        /**
         * Returns the block size of a single dynamic bucket instance at compile time.
         */
		template<typename TBucket>
        struct GetDynamicBucketBlockSize : std::integral_constant<size_t, TBucket::s_blockSize>
        {};

        /**
         * Returns the block count of a single dynamic bucket instance at compile time.
         */
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

        /**
         * @brief Allocates the required amount of bytes from a specific bucket of a dynamic memory
         * pool, triggering a resize of the pool if needed.
         * 
         * @param bytes the amount of bytes that we want to allocate.
         * @param index of the bucket that we want to allocate memory to.
         * @return pointer to allocated data.
         */
		template<typename... TBuckets>
        [[nodiscard]] void* AllocateFromDynamicBucket(size_t bytes, size_t index)
        {
            DynamicMemoryPool<TBuckets...>& pool = GetDynamicMemoryPool<TBuckets...>();
            if (index >= DynamicBucketCount<TBuckets...>)
            {
                throw std::bad_alloc();
            }

            bucket_container_t& bucket = pool[index];
            if (void* ptr = bucket.allocate(bytes); ptr != nullptr)
            {
                return ptr;
            }

            throw std::bad_alloc();
        }

        /**
         * @brief Deallocates memory from a dynamic memory pool. 
         * 
         * @param ptr to the memory that we want to deallocate. 
         * @param bytes amount of memory that we want to deallocate.
         */
		template<typename... TBuckets>
        void DeallocateFromDynamicBucket(void* ptr, size_t bytes) noexcept 
        {
            DynamicMemoryPool<TBuckets...>& pool = GetDynamicMemoryPool<TBuckets...>();
            for (bucket_container_t& bucket : pool)
            {
                if (bucket.belongs_to_this(ptr))
                {
                    bucket.deallocate(ptr, bytes);
                    break;
                }
            }
        }

        /**
         * @brief Returns true if a dynamic memory pool defined by the provided bucket descriptors 
         * does actually exist at runtime.
         * 
         * @tparam TBuckets the descriptors of the buckets that define the memory pool. 
         * @return true if the memory poll is defined.
         */
        template<typename... TBuckets>
        constexpr bool IsDynamicMemoryPoolDefined() noexcept 
        {
            return DynamicBucketCount<TBuckets...> != 0;
        }

        /**
         * @brief Initializes a dynamic memory pool defined by the provided bucket descriptors
         * 
         * @tparam TBuckets the descriptors of the buckets that define the memory pool. 
         * @return true if the memory pool is effectively initialized.
         */
        template<typename... TBuckets>
        bool InitializeDynamicMemoryPool() noexcept 
        {
            (void) GetDynamicMemoryPool<TBuckets...>();
            return IsDynamicMemoryPoolDefined<TBuckets...>();
        }
	}
}