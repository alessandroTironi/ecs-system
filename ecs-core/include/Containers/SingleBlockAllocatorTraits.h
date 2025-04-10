#pragma once 

#include <vector>

namespace ecs
{
	template <typename AllocatorType, typename T, typename = void>
	struct SingleBlockAllocatorTrait 
	{
		static constexpr bool IsSingleBlockAllocator = false;
	};

	// Detect if a type has begin() and end() methods using SFINAE
	template <typename AllocatorType, typename T>
	struct SingleBlockAllocatorTrait<
		AllocatorType,
		T,
		std::void_t<
			decltype(std::declval<AllocatorType>().AllocateBlock()),
			decltype(std::declval<AllocatorType>().FreeBlock(std::declval<size_t>())),
			decltype(std::declval<T&>() = std::declval<AllocatorType>()[std::declval<size_t>()])
		>
	> 
	{
		static constexpr bool IsSingleBlockAllocator = true;
		
		static size_t AllocateBlock(AllocatorType& allocator) 
		{
			return allocator.AllocateBlock();
		}

		static void FreeBlock(AllocatorType& allocator, const size_t index)
		{
			allocator.FreeBlock(index);
		}

		static T& GetBlock(AllocatorType& allocator, const size_t index)
		{
			return allocator[index];
		}
	};
}