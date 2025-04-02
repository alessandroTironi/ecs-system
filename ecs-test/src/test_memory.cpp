#include <gtest/gtest.h>
#include "Containers/memory.h"
#include "Containers/SingleBlockFreeListAllocator.h"

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
};

TEST_F(TestMemory, TestSingleBlockAllocatorConstructorAndDestructor)
{
    ecs::SingleBlockFreeListAllocator<int, 8> intAllocator;
    EXPECT_EQ(intAllocator.usedCount(), 0);
    EXPECT_EQ(intAllocator.freeCount(), 8);
    EXPECT_EQ(intAllocator.capacity(), 8);
}

TEST_F(TestMemory, TestSingleBlockAllocation)
{
    ecs::SingleBlockFreeListAllocator<int, 8> intAllocator;
    size_t index;
    ASSERT_NO_THROW(index = intAllocator.AllocateBlock());

    EXPECT_EQ(intAllocator.usedCount(), 1);
    EXPECT_EQ(intAllocator.freeCount(), 7);
}

TEST_F(TestMemory, TestSingleBlockDeallocation)
{
    ecs::SingleBlockFreeListAllocator<int, 8> intAllocator;
    const size_t index = intAllocator.AllocateBlock();

    intAllocator.FreeBlock(index);
    EXPECT_EQ(intAllocator.usedCount(), 0);
    EXPECT_EQ(intAllocator.freeCount(), 8);
}

TEST_F(TestMemory, TestSingleBlockAccess)
{
    ecs::SingleBlockFreeListAllocator<int, 8> intAllocator;
    const size_t index = intAllocator.AllocateBlock();
    intAllocator[index] = 5;

    EXPECT_EQ(intAllocator[index], 5);
}

TEST_F(TestMemory, TestSingleBlockAllocatorResize)
{
    ecs::SingleBlockFreeListAllocator<int, 2> intAllocator;
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