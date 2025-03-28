#include <gtest/gtest.h>
#include "Core/CompactString.h"

using ::testing::Test;

class TestCompactString : public Test 
{
public:
    void SetUp() override
    {
        m_emptyString = "";
        m_shortString = std::string("short");
        m_longString = std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    }

    void TearDown() override
    {

    }
    
protected:
    ecs::compact_string<32> m_emptyString;
    ecs::compact_string<32> m_shortString;
    ecs::compact_string<32> m_longString;
};

TEST_F(TestCompactString, EmptyString)
{
    EXPECT_EQ(m_emptyString.size(), 0);
    EXPECT_EQ(m_emptyString.capacity(), 32);
}

TEST_F(TestCompactString, ShortString)
{
    EXPECT_EQ(m_shortString.size(), 5);
    EXPECT_EQ(m_shortString.capacity(), 32);
}

TEST_F(TestCompactString, LongString)
{
    EXPECT_EQ(m_longString.size(), 31);
    EXPECT_EQ(m_longString.capacity(), 32);
}

TEST_F(TestCompactString, StringConstructor)
{
    std::string str = "Hello, World!";
    ecs::compact_string<32> compactStr(str);
    EXPECT_EQ(compactStr.size(), str.size());
    EXPECT_EQ(compactStr.capacity(), 32);
    EXPECT_EQ(compactStr, str);
}

TEST_F(TestCompactString, CharConstructor)
{
    const char* str = "Hello, World!";
    ecs::compact_string<32> compactStr(str);
    EXPECT_EQ(compactStr.size(), strlen(str));
    EXPECT_EQ(compactStr.capacity(), 32);
    EXPECT_EQ(compactStr, str);
}

TEST_F(TestCompactString, CopyConstructor)
{
    ecs::compact_string<32> compactStr = m_shortString;
    EXPECT_EQ(compactStr.size(), m_shortString.size());
    EXPECT_EQ(compactStr.capacity(), m_shortString.capacity());
    EXPECT_EQ(compactStr, m_shortString);
}

TEST_F(TestCompactString, AssignmentOperator)
{
    ecs::compact_string<32> compactStr;
    compactStr = m_shortString;
    EXPECT_EQ(compactStr.size(), m_shortString.size());
    EXPECT_EQ(compactStr.capacity(), m_shortString.capacity());
}

TEST_F(TestCompactString, EqualityOperator)
{
    ecs::compact_string<32> compactStr1 = "Hello";
    ecs::compact_string<32> compactStr2 = "Hello";
    EXPECT_EQ(compactStr1, compactStr2);
}

TEST_F(TestCompactString, InequalityOperator)
{
    ecs::compact_string<32> compactStr1 = "Hello";
    ecs::compact_string<32> compactStr2 = "World";
    EXPECT_NE(compactStr1, compactStr2);
}

TEST_F(TestCompactString, StringAssignment)
{
    std::string str = "Hello, World!";
    m_emptyString = str;
    EXPECT_EQ(m_emptyString, str);
}

TEST_F(TestCompactString, CharAssignment)
{
    const char* str = "Hello, World!";
    m_emptyString = str;
    EXPECT_EQ(m_emptyString, str);
}

TEST_F(TestCompactString, Hashing)
{
    ecs::compact_string<32> compactStr1 = "Hello";
    ecs::compact_string<32> compactStr2 = "Hello";
    EXPECT_EQ(std::hash<ecs::compact_string<32>>()(compactStr1), std::hash<ecs::compact_string<32>>()(compactStr2));
}


