#include <gtest/gtest.h>
#include <functional>
#include "Core/Delegates.h"

using ::testing::Test;

class TestDelegates : public Test 
{
public:
    void SetUp() override 
    {
        m_counter = 0;
    }

    void TearDown() override 
    {

    }

protected:
    int m_counter = 0;
};

TEST_F(TestDelegates, TestVoidDelegate)
{
    ecs::delegate_t<void> voidDelegate = 
        ecs::delegate_t<void>::create([&]() {m_counter += 1; });

    voidDelegate();
    ASSERT_EQ(m_counter, 1);
}

TEST_F(TestDelegates, TestVoidDelegateWithArguments)
{
    ecs::delegate_t<void, int> delegate =
        ecs::delegate_t<void, int>::create([&](int i) { m_counter += i; });

    delegate(5);
    ASSERT_EQ(m_counter, 5);
}

TEST_F(TestDelegates, TestDelegateWithReturnType)
{
    ecs::delegate_t<int> delegate =
        ecs::delegate_t<int>::create([&]() { return 10; });

    ASSERT_EQ(delegate(), 10);
}

TEST_F(TestDelegates, TestDelegateWithReturnTypeAndArguments)
{
    ecs::delegate_t<int, int> delegate =
        ecs::delegate_t<int, int>::create([&](int i) { return 10 + i; });

    ASSERT_EQ(delegate(5), 15);
}