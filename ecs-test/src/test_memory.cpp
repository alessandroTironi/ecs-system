#include <gtest/gtest.h>
#include "Containers/memory.h"
#include "Containers/SingleBlockChunkAllocator.h"

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
        ASSERT_NO_THROW(index = intAllocator.AllocateBlock());

        EXPECT_EQ(intAllocator.usedCount(), 1);
        EXPECT_EQ(intAllocator.freeCount(), prevFreeCount - 1);
    }

    template<typename Allocator>
    void TestSingleDeallocation()
    {
        Allocator intAllocator;
        const size_t index = intAllocator.AllocateBlock();

        const size_t prevFreeCount = intAllocator.freeCount();
        intAllocator.FreeBlock(index);
        EXPECT_EQ(intAllocator.usedCount(), 0);
        EXPECT_EQ(intAllocator.freeCount(), prevFreeCount + 1);
    }

    template<typename Allocator>
    void TestAccess()
    {
        Allocator intAllocator;
        const size_t index = intAllocator.AllocateBlock();
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
            intAllocator.AllocateBlock();
            intAllocator[i] = 10 * i;
        }

        intAllocator.AllocateBlock();
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