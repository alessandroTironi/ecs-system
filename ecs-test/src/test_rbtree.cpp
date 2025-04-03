#include <gtest/gtest.h>
#include "Containers/sm_rbtree.h"

using ::testing::Test;

class TestSharedMemoryRBTree : public Test 
{
    virtual void SetUp() override
    {

    }

    virtual void TearDown() override 
    {

    }
};

TEST_F(TestSharedMemoryRBTree, TestFirstAllocation)
{
    ecs::sm_rbtree<int> tree;
    EXPECT_EQ(tree.size(), 0);
}

TEST_F(TestSharedMemoryRBTree, TestInsert)
{
    ecs::sm_rbtree<int> tree;
    ASSERT_NO_THROW(tree.insert(1));
    EXPECT_EQ(tree.size(), 1);
}

TEST_F(TestSharedMemoryRBTree, TestInsertIdempotent)
{
    ecs::sm_rbtree<int> tree;
    tree.insert(1);
    tree.insert(1);
    EXPECT_EQ(tree.size(), 1);
}

TEST_F(TestSharedMemoryRBTree, TestInsertNonRootIdempotent)
{
    ecs::sm_rbtree<int> tree;
    tree.insert(1);

    tree.insert(2);
    EXPECT_EQ(tree.size(), 2);
    
    tree.insert(2);
    EXPECT_EQ(tree.size(), 2);
}

TEST_F(TestSharedMemoryRBTree, TestErase)
{
    ecs::sm_rbtree<int> tree;
    ASSERT_NO_THROW(tree.erase(1));

    tree.insert(1);
    ASSERT_NO_THROW(tree.erase(1));
}

TEST_F(TestSharedMemoryRBTree, TestClear)
{
    ecs::sm_rbtree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    ASSERT_NO_THROW(tree.clear());
    EXPECT_EQ(tree.size(), 0);
}

TEST_F(TestSharedMemoryRBTree, TestFind)
{
    ecs::sm_rbtree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    EXPECT_EQ(tree.find(4), tree.end());
    EXPECT_NE(tree.find(1), tree.end());
}

TEST_F(TestSharedMemoryRBTree, TestIterator)
{
    ecs::sm_rbtree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    auto it = tree.begin();
    EXPECT_EQ(it.value(), 1);
    ++it;
    EXPECT_EQ(it.value(), 2);
    ++it;
    EXPECT_EQ(it.value(), 3);
    ++it;
    EXPECT_EQ(it, tree.end());
}

TEST_F(TestSharedMemoryRBTree, TestOrder)
{
    ecs::sm_rbtree<int> tree;
    tree.insert(5);
    tree.insert(4);
    tree.insert(3);
    tree.insert(2);
    tree.insert(1);

    int prev = -1;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        EXPECT_TRUE(it.value() > prev);
        prev = it.value();
    }
}
