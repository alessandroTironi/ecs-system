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