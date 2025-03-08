#include <gtest/gtest.h>
#include <limits>
#include "IDGenerator.h"

using ::testing::Test;

class TestIDGenerator : public Test 
{
public:
    ecs::IDGenerator<unsigned long>* m_ulongIDGenerator;
    ecs::IDGenerator<unsigned char>* m_charIDGenerator;

    void SetUp() override 
    {
        m_ulongIDGenerator = new ecs::IDGenerator<unsigned long>();
        m_charIDGenerator = new ecs::IDGenerator<unsigned char>();
    }

    void TearDown() override 
    {
        delete m_ulongIDGenerator;
        delete m_charIDGenerator;
    }
};

TEST_F(TestIDGenerator, TestFirstGeneration)
{
    const unsigned long firstUlongID = m_ulongIDGenerator->GenerateNewUniqueID();
    EXPECT_EQ(firstUlongID, 0) << "First generated ID is not zero";
}

TEST_F(TestIDGenerator, TestSubsequentGenerations)
{
    EXPECT_EQ(m_ulongIDGenerator->GenerateNewUniqueID(), 0);
    EXPECT_EQ(m_ulongIDGenerator->GenerateNewUniqueID(), 1);
}

TEST_F(TestIDGenerator, TestConcurrentGenerations)
{
    EXPECT_EQ(m_ulongIDGenerator->GenerateNewUniqueID(), 0);
    EXPECT_EQ(m_charIDGenerator->GenerateNewUniqueID(), 0);
}

TEST_F(TestIDGenerator, TestOverflow)
{
    static const auto GenerateInfiniteUniqueIDs = [](ecs::IDGenerator<unsigned char>* generator)
    {
        const unsigned char maxID = std::numeric_limits<unsigned char>::max();
        for (unsigned char id = 0; id < 1000; ++id)
        {
            generator->GenerateNewUniqueID();
        }
    };

    ASSERT_THROW(GenerateInfiniteUniqueIDs(m_charIDGenerator), std::overflow_error);
}

TEST_F(TestIDGenerator, TestReset)
{
    m_ulongIDGenerator->GenerateNewUniqueID();
    m_ulongIDGenerator->GenerateNewUniqueID();
    m_ulongIDGenerator->GenerateNewUniqueID();

    m_ulongIDGenerator->Reset();
    EXPECT_EQ(m_ulongIDGenerator->GenerateNewUniqueID(), 0);
}