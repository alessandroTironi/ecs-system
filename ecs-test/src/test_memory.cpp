#include <gtest/gtest.h>
#include "Containers/memory.h"
#include "Containers/SingleBlockFreeListAllocator.h"
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
        EXPECT_EQ(intAllocator.freeCount(), 8);
        EXPECT_EQ(intAllocator.capacity(), 8);
    }

    template<typename Allocator>
    void TestSingleAllocation()
    {
        Allocator intAllocator;
        size_t index;
        ASSERT_NO_THROW(index = intAllocator.AllocateBlock());

        EXPECT_EQ(intAllocator.usedCount(), 1);
        EXPECT_EQ(intAllocator.freeCount(), 7);
    }

    template<typename Allocator>
    void TestSingleDeallocation()
    {
        Allocator intAllocator;
        const size_t index = intAllocator.AllocateBlock();

        intAllocator.FreeBlock(index);
        EXPECT_EQ(intAllocator.usedCount(), 0);
        EXPECT_EQ(intAllocator.freeCount(), 8);
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
    void TestResize()
    {
        Allocator intAllocator;
        intAllocator.AllocateBlock();
        intAllocator.AllocateBlock();
        intAllocator[0] = 10;
        intAllocator[1] = 100;

        intAllocator.AllocateBlock();
        intAllocator[2] = 1000;
        EXPECT_EQ(intAllocator.capacity(), 4);
        EXPECT_EQ(intAllocator[0], 10);
        EXPECT_EQ(intAllocator[1], 100);
        EXPECT_EQ(intAllocator[2], 1000);
    }
};

TEST_F(TestMemory, TestSingleBlockAllocatorConstructorAndDestructor)
{
    TestSingleConstructorAndDestructor<ecs::SingleBlockFreeListAllocator<int, 8>>();
}

TEST_F(TestMemory, TestSingleBlockAllocation)
{
    TestSingleAllocation<ecs::SingleBlockFreeListAllocator<int, 8>>();
}

TEST_F(TestMemory, TestSingleBlockDeallocation)
{
    TestSingleDeallocation<ecs::SingleBlockFreeListAllocator<int, 8>>();
}

TEST_F(TestMemory, TestSingleBlockAccess)
{
    TestAccess<ecs::SingleBlockFreeListAllocator<int, 8>>();
}

TEST_F(TestMemory, TestSingleBlockAllocatorResize)
{
    TestResize<ecs::SingleBlockFreeListAllocator<int, 2>>();
}