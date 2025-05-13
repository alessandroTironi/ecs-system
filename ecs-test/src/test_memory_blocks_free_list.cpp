#include <gtest/gtest.h>
#include "Containers/MemoryBlocksFreeList.h"

using ::testing::Test;

class MemoryBlocksFreeListTest : public Test
{
public:
    void SetUp() override 
    {

    }

    void TearDown() override 
    {

    }

protected:
    ecs::memory_pool::memory_blocks_free_list m_freeList;
    ecs::memory_pool::FreeMemoryTracker m_memoryTracker;
};

TEST_F(MemoryBlocksFreeListTest, Constructor)
{
    EXPECT_EQ(m_freeList.size(), 0);
}

TEST_F(MemoryBlocksFreeListTest, TestAddFirstBlock)
{
    m_freeList.add(0, 10);
    EXPECT_EQ(m_freeList.size(), 1);
    EXPECT_EQ(m_freeList[0].first_index(), 0);
    EXPECT_EQ(m_freeList[0].last_index(), 9);
    EXPECT_EQ(m_freeList[0].size(), 10);
}

TEST_F(MemoryBlocksFreeListTest, TestAddBeforeHeadMerge)
{
    m_freeList.add(10, 10);
    m_freeList.add(0, 10);
    EXPECT_EQ(m_freeList.size(), 1);
    EXPECT_EQ(m_freeList[0].first_index(), 0);
    EXPECT_EQ(m_freeList[0].last_index(), 19);
    EXPECT_EQ(m_freeList[0].size(), 20);
}

TEST_F(MemoryBlocksFreeListTest, TestAddBeforeHeadMergeOverlap)
{
    m_freeList.add(10, 10);
    m_freeList.add(0, 15);
    EXPECT_EQ(m_freeList.size(), 1);
    ecs::memory_pool::memory_blocks_free_list::block_t b0 = m_freeList[0];
    EXPECT_EQ(b0.first_index(), 0);
    EXPECT_EQ(b0.last_index(), 19);
    EXPECT_EQ(b0.size(), 20);
}

TEST_F(MemoryBlocksFreeListTest, TestAddBeforeHeadNoMerging)
{
    m_freeList.add(20, 10);
    m_freeList.add(0, 10);
    auto b0 = m_freeList[0];
    auto b1 = m_freeList[1];
    EXPECT_EQ(m_freeList.size(), 2);
    EXPECT_EQ(b0.first_index(), 0);
    EXPECT_EQ(b0.last_index(), 9);
    EXPECT_EQ(b0.size(), 10);
    EXPECT_EQ(b1.first_index(), 20);
    EXPECT_EQ(b1.last_index(), 29);
    EXPECT_EQ(b1.size(), 10);
}

TEST_F(MemoryBlocksFreeListTest, TestAddAfterHeadMergeWithPrevious)
{
    m_freeList.add(0, 10);
    m_freeList.add(20, 10);
    m_freeList.add(30, 10);
    EXPECT_EQ(m_freeList.size(), 2);
    EXPECT_EQ(m_freeList[0].first_index(), 0);
    EXPECT_EQ(m_freeList[0].last_index(), 9);
    EXPECT_EQ(m_freeList[0].size(), 10);
    EXPECT_EQ(m_freeList[1].first_index(), 20);
    EXPECT_EQ(m_freeList[1].last_index(), 39);
    EXPECT_EQ(m_freeList[1].size(), 20);
}

TEST_F(MemoryBlocksFreeListTest, TestAddAfterHeadMergeWithNext)
{
    m_freeList.add(0, 10);
    m_freeList.add(20, 10);
    m_freeList.add(40, 10);
    m_freeList.add(60, 10);
    m_freeList.add(55, 5);

    EXPECT_EQ(m_freeList.size(), 4);
    EXPECT_EQ(m_freeList[0].first_index(), 0);
    EXPECT_EQ(m_freeList[0].last_index(), 9);
    EXPECT_EQ(m_freeList[0].size(), 10);
    EXPECT_EQ(m_freeList[1].first_index(), 20);
    EXPECT_EQ(m_freeList[1].last_index(), 29);
    EXPECT_EQ(m_freeList[1].size(), 10);
    EXPECT_EQ(m_freeList[2].first_index(), 40);
    EXPECT_EQ(m_freeList[2].last_index(), 49);
    EXPECT_EQ(m_freeList[2].size(), 10);
    EXPECT_EQ(m_freeList[3].first_index(), 55);
    EXPECT_EQ(m_freeList[3].last_index(), 69);
    EXPECT_EQ(m_freeList[3].size(), 15);
}

TEST_F(MemoryBlocksFreeListTest, TestAddAfterHeadMergeBothSides)
{
    m_freeList.add(0, 10);
    m_freeList.add(20, 10);
    m_freeList.add(40, 10);
    m_freeList.add(60, 10);
    auto b0 = m_freeList[0];
    auto b1 = m_freeList[1];
    auto b2 = m_freeList[2];
    auto b3 = m_freeList[3];

    EXPECT_EQ(m_freeList.size(), 4);
    EXPECT_EQ(b0.first_index(), 0);
    EXPECT_EQ(b0.last_index(), 9);
    EXPECT_EQ(b0.size(), 10);
    EXPECT_EQ(b1.first_index(), 20);
    EXPECT_EQ(b1.last_index(), 29);
    EXPECT_EQ(b1.size(), 10);
    EXPECT_EQ(b2.first_index(), 40);
    EXPECT_EQ(b2.last_index(), 49);
    EXPECT_EQ(b2.size(), 10);
    EXPECT_EQ(b3.first_index(), 60);
    EXPECT_EQ(b3.last_index(), 69);
    EXPECT_EQ(b3.size(), 10);

    m_freeList.add(50, 10);

    b0 = m_freeList[0];
    b1 = m_freeList[1];
    b2 = m_freeList[2];
    EXPECT_EQ(m_freeList.size(), 3);
    EXPECT_EQ(b0.first_index(), 0);
    EXPECT_EQ(b0.last_index(), 9);
    EXPECT_EQ(b0.size(), 10);
    EXPECT_EQ(b1.first_index(), 20);
    EXPECT_EQ(b1.last_index(), 29);
    EXPECT_EQ(b1.size(), 10);
    EXPECT_EQ(b2.first_index(), 40);
    EXPECT_EQ(b2.last_index(), 69);
    EXPECT_EQ(b2.size(), 30);
}

TEST_F(MemoryBlocksFreeListTest, TestRemoveBlockNoWaste)
{
    m_freeList.add(0, 10);
    m_freeList.add(20, 10);
    m_freeList.add(40, 10);
    m_freeList.add(60, 10);

    std::optional<size_t> index = m_freeList.find_and_remove(10);
    EXPECT_EQ(index.has_value(), true);
    EXPECT_EQ(m_freeList.size(), 3);
}

TEST_F(MemoryBlocksFreeListTest, TestRemoveBlockWithWaste)
{
    m_freeList.add(0, 3);
    m_freeList.add(20, 7);
    m_freeList.add(40, 10);
    m_freeList.add(60, 20);

    std::optional<size_t> index = m_freeList.find_and_remove(6);
    EXPECT_EQ(index.has_value(), true);
    EXPECT_EQ(index.value(), 20);
    EXPECT_EQ(m_freeList.size(), 4);
}