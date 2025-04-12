#include <gtest/gtest.h>
#include "Containers/memory.h"
#include "Containers/SingleBlockChunkAllocator.h"
#include "Containers/SingleBlockAllocatorTraits.h"

using ::testing::Test; 

class TestMemory : public Test 
{
protected:
    virtual void SetUp() override 
    {

    }

    virtual void TearDown() override 
    {

    }

    template<typename Allocator>
    void TestSingleConstructorAndDestructor()
    {
        Allocator intAllocator;
        EXPECT_EQ(intAllocator.usedCount(), 0);
    }

    template<typename Allocator>
    void TestSingleAllocation()
    {
        Allocator intAllocator;
        size_t index;
        const size_t prevFreeCount = intAllocator.freeCount();
        index = ecs::SingleBlockAllocatorTrait<Allocator, int>::AllocateBlock(intAllocator);

        EXPECT_EQ(intAllocator.usedCount(), 1);
        EXPECT_EQ(intAllocator.freeCount(), prevFreeCount - 1);
    }

    template<typename Allocator>
    void TestSingleDeallocation()
    {
        Allocator intAllocator;
        const size_t index = ecs::SingleBlockAllocatorTrait<Allocator, int>::AllocateBlock(intAllocator);

        const size_t prevFreeCount = intAllocator.freeCount();
        ecs::SingleBlockAllocatorTrait<Allocator, int>::FreeBlock(intAllocator, index);
        EXPECT_EQ(intAllocator.usedCount(), 0);
        EXPECT_EQ(intAllocator.freeCount(), prevFreeCount + 1);
    }

    template<typename Allocator>
    void TestAccess()
    {
        Allocator intAllocator;
        const size_t index = ecs::SingleBlockAllocatorTrait<Allocator, int>::AllocateBlock(intAllocator);
        intAllocator[index] = 5;

        EXPECT_EQ(intAllocator[index], 5);
    }

    template<typename Allocator>
    void TestResize(size_t resizeThreshold)
    {
        resizeThreshold = resizeThreshold < 2? 2 : resizeThreshold;
        Allocator intAllocator;
        
        for (size_t i = 0; i < resizeThreshold; ++i)
        {
            ecs::SingleBlockAllocatorTrait<Allocator, int>::AllocateBlock(intAllocator);
            intAllocator[i] = 10 * i;
        }

        ecs::SingleBlockAllocatorTrait<Allocator, int>::AllocateBlock(intAllocator);
        intAllocator[resizeThreshold] = 1000;
        EXPECT_EQ(intAllocator[0], 0);
        EXPECT_EQ(intAllocator[1], 10);
        EXPECT_EQ(intAllocator[resizeThreshold], 1000);
    }
};

TEST_F(TestMemory, TestChunkAllocatorConstructorAndDestructor)
{
    TestSingleConstructorAndDestructor<ecs::SingleBlockChunkAllocator<int>>();
}

TEST_F(TestMemory, TestChunkAllocatorAllocation)
{
    TestSingleAllocation<ecs::SingleBlockChunkAllocator<int>>();
}

TEST_F(TestMemory, TestChunkAllocatorSingleDeallocation)
{
    TestSingleDeallocation<ecs::SingleBlockChunkAllocator<int>>();
}

TEST_F(TestMemory, TestChunkAllocatorAccess)
{
    TestAccess<ecs::SingleBlockChunkAllocator<int>>();
}

TEST_F(TestMemory, TestChunkAllocatorResize)
{
    TestResize<ecs::SingleBlockChunkAllocator<int>>(64);
}