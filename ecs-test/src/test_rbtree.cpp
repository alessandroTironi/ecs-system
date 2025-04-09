#include <gtest/gtest.h>
#include "Containers/rbtree.h"
#include <iostream>

using ::testing::Test;


class TestRBTree : public Test 
{
    virtual void SetUp() override
    {

    }

    virtual void TearDown() override 
    {

    }

protected:
    void TestInsertionSequence(std::initializer_list<int> values)
    {
        for (const int& value : values)
        {
            m_tree.insert(value);
            ASSERT_TRUE(m_tree.is_valid_tree())
                << "Tree lost balance after inserting " << value;
        }
    }

    void TestInsertionSequenceFollowedByDeletionSequence(std::initializer_list<int> valuesToInsert,
        std::initializer_list<int> valuesToErase, bool logSteps = false)
    {
        for (const int& value : valuesToInsert)
        {
            const std::string before = m_tree.to_string();
            m_tree.insert(value);
            const std::string after = m_tree.to_string();
            if (logSteps)
            {
                std::cout << "Tree after inserting " << value << ": " << std::endl << after << std::endl;
            }

            ASSERT_TRUE(m_tree.is_valid_tree())
                << "Tree lost balance after inserting " << value << std::endl 
                << "Before was: " << std::endl << before << std::endl 
                << "After was: " << std::endl << after << std::endl;
        }

        for (const int& value : valuesToErase)
        {
            const std::string before = m_tree.to_string();
            m_tree.erase(value);
            const std::string after = m_tree.to_string();
            if (logSteps)
            {
                std::cout << "Tree after erasing " << value << ": " << std::endl << after << std::endl;
            }

            ASSERT_TRUE(m_tree.is_valid_tree())
                << "Tree lost balance after erasing " << value << std::endl 
                << "Before was: " << std::endl << before << std::endl 
                << "After was: " << std::endl << after << std::endl;
        }
    }

    ecs::rbtree<int> m_tree;
};

TEST_F(TestRBTree, TestFirstAllocation)
{
    ecs::rbtree<int> tree;
    EXPECT_EQ(tree.size(), 0);
}

TEST_F(TestRBTree, TestValidEmptyTree)
{
    ecs::rbtree<int> tree;
    EXPECT_TRUE(tree.is_valid_tree());
}

TEST_F(TestRBTree, TestInsert)
{
    ecs::rbtree<int> tree;
    ASSERT_NO_THROW(tree.insert(1));
    EXPECT_EQ(tree.size(), 1);
    EXPECT_TRUE(tree.is_valid_tree());
}

TEST_F(TestRBTree, TestInsertIdempotent)
{
    ecs::rbtree<int> tree;
    tree.insert(1);
    tree.insert(1);
    EXPECT_EQ(tree.size(), 1);
    EXPECT_TRUE(tree.is_valid_tree());
}

TEST_F(TestRBTree, TestInsertNonRootIdempotent)
{
    ecs::rbtree<int> tree;
    tree.insert(1);

    tree.insert(2);
    EXPECT_EQ(tree.size(), 2);
    EXPECT_TRUE(tree.is_valid_tree());
    
    tree.insert(2);
    EXPECT_EQ(tree.size(), 2);
    EXPECT_TRUE(tree.is_valid_tree());
}

TEST_F(TestRBTree, TestSimpleRotations)
{
    TestInsertionSequence({ 10, 5, 15, 3, 7, 12, 20 });
}

TEST_F(TestRBTree, TestLeftLeftCase)
{
    TestInsertionSequence({30, 20, 10});
}

TEST_F(TestRBTree, TestRightRightCase)
{
    TestInsertionSequence({ 10, 20, 30 });
}

TEST_F(TestRBTree, TestLeftRightCase)
{
    TestInsertionSequence({ 30, 10, 20 });
}

TEST_F(TestRBTree, TestRightLeftCase)
{
    TestInsertionSequence({ 10, 30, 20 });
}

TEST_F(TestRBTree, TestRedUncleCase)
{
    TestInsertionSequence({ 20, 10, 30, 5, 15});
}

TEST_F(TestRBTree, TestBlackUncleWithZigZagPattern)
{
    TestInsertionSequence({ 20, 10, 5, 15 });
}

TEST_F(TestRBTree, TestRootRecoloring)
{
    TestInsertionSequence({ 10, 5, 15, 3, 7, 12, 20, 2});
}

TEST_F(TestRBTree, TestConsecutiveRebalancing)
{
    TestInsertionSequence({ 50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 55, 1});
}

TEST_F(TestRBTree, TestDeepTree)
{
    TestInsertionSequence({ 50, 25, 75, 12, 37, 62, 87, 6, 18, 31, 43, 56, 68, 81, 93, 3, 9, 15, 21, 28, 34, 40, 46, 53, 59, 65, 71, 78, 84, 90, 96 });
}

TEST_F(TestRBTree, TestAlternatingPatterns)
{
    TestInsertionSequence({ 100, 50, 150, 25, 75, 125, 175, 12, 37, 62, 87, 112, 137, 162, 187 });
}

TEST_F(TestRBTree, TestCascadingRebalance)
{
    TestInsertionSequence({ 16, 8, 24, 4, 12, 20, 28, 2, 6, 10, 14, 18, 22, 26, 30, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31 });
}

TEST_F(TestRBTree, TestNearlyCompleteTreeWithUnbalancedInsertion)
{
    TestInsertionSequence({ 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15 });
}

TEST_F(TestRBTree, TestPathologicalIncreasingSequence)
{
    TestInsertionSequence({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
}

TEST_F(TestRBTree, TestPathologicalDecreasingSequence)
{
    TestInsertionSequence({ 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 });
}

TEST_F(TestRBTree, TestErase)
{
    ecs::rbtree<int> tree;
    ASSERT_NO_THROW(tree.erase(1));
    EXPECT_TRUE(tree.is_valid_tree());

    tree.insert(1);
    ASSERT_NO_THROW(tree.erase(1));
    EXPECT_TRUE(tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseRedLeafNode)
{
    m_tree.insert(10);
    m_tree.insert(5);
    m_tree.insert(15);
    m_tree.insert(3);

    m_tree.erase(3);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseBlackNodeWithRedChild)
{
    m_tree.insert(20);
    m_tree.insert(10);
    m_tree.insert(30);
    m_tree.insert(5);
    m_tree.insert(15);

    m_tree.erase(10);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseBlackLeafNode)
{
    m_tree.insert(20);
    m_tree.insert(10);
    m_tree.insert(30);
    m_tree.insert(5);
    m_tree.insert(25);
    m_tree.insert(40);

    m_tree.erase(5);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseNodeWithRedSibling)
{
    m_tree.insert(20);
    m_tree.insert(10);
    m_tree.insert(30);
    m_tree.insert(5);
    m_tree.insert(15);
    m_tree.insert(40);
    m_tree.insert(25);

    m_tree.erase(5);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseWithBlackSiblingAndBlackNephews)
{
    m_tree.insert(20);
    m_tree.insert(10);
    m_tree.insert(30);
    m_tree.insert(5);
    m_tree.insert(15);
    m_tree.insert(25);
    m_tree.insert(40);
    m_tree.insert(3);
    m_tree.insert(7);

    m_tree.erase(3);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseWithRedNearNephew)
{
    m_tree.insert(20);
    m_tree.insert(10);
    m_tree.insert(30);
    m_tree.insert(5);
    m_tree.insert(15);
    m_tree.insert(25);
    m_tree.insert(40);
    m_tree.insert(17); 
    
    m_tree.erase(25);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseWithRedFarNephew)
{
    TestInsertionSequenceFollowedByDeletionSequence(
        { 20, 10, 30, 5, 15, 25, 40, 13 },
        { 5 }
    );
}

TEST_F(TestRBTree, TestEraseRootFromThreeNodeTree)
{
    m_tree.insert(20);
    m_tree.insert(10);
    m_tree.insert(30);
    
    m_tree.erase(20);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseRequiringMultipleRebalancing)
{
    TestInsertionSequence({50, 25, 75, 12, 37, 62, 87, 6, 18, 31, 43, 56, 68, 81, 93});

    m_tree.erase(25);
    EXPECT_TRUE(m_tree.is_valid_tree());

    m_tree.erase(75);
    EXPECT_TRUE(m_tree.is_valid_tree());

    m_tree.erase(50);
    EXPECT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestSequentialEraseWithRebalancing)
{
    TestInsertionSequenceFollowedByDeletionSequence(
        {50, 25, 75, 12, 37, 62, 87, 6, 18, 31, 43, 56, 68, 81, 93},
        {12, 87, 25, 56, 93, 37, 75, 6, 62, 43, 50, 31, 68, 18, 81}
    );

    EXPECT_EQ(m_tree.size(), 0);
}

TEST_F(TestRBTree, TestEraseNodeWithBothChildren)
{
    m_tree.insert(50);
    m_tree.insert(25);
    m_tree.insert(75);
    m_tree.insert(12);
    m_tree.insert(37);
    m_tree.insert(62);
    m_tree.insert(87);
    
    // Delete 25, which has both 12 and 37 as children
    m_tree.erase(25);
    ASSERT_TRUE(m_tree.is_valid_tree());
    
    // Delete 75, which has both 62 and 87 as children
    m_tree.erase(75);
    ASSERT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestEraseCausingFixupToRoot)
{
    m_tree.insert(10);
    m_tree.insert(5);
    m_tree.insert(20);
    m_tree.insert(3);
    m_tree.insert(7);
    m_tree.insert(15);
    m_tree.insert(25);
    
    // Delete in sequence to force cascading fixup
    m_tree.erase(3);
    ASSERT_TRUE(m_tree.is_valid_tree());
    
    m_tree.erase(7);
    ASSERT_TRUE(m_tree.is_valid_tree());
    
    m_tree.erase(5); 
    ASSERT_TRUE(m_tree.is_valid_tree());
}

TEST_F(TestRBTree, TestClear)
{
    ecs::rbtree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    ASSERT_NO_THROW(tree.clear());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.is_valid_tree());
}

TEST_F(TestRBTree, TestFind)
{
    ecs::rbtree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    EXPECT_EQ(tree.find(4), tree.end());
    EXPECT_NE(tree.find(1), tree.end());
}

TEST_F(TestRBTree, TestIterator)
{
    ecs::rbtree<int> tree;
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

TEST_F(TestRBTree, TestOrder)
{
    ecs::rbtree<int> tree;
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