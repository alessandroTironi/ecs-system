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
    DEFINE_VOID_DELEGATE(IncrementCounter);
    IncrementCounter delegate = IncrementCounter::create([&]() {m_counter += 1; });

    delegate.invoke();
    ASSERT_EQ(m_counter, 1);
}

TEST_F(TestDelegates, TestVoidDelegateWithArguments)
{
    DEFINE_VOID_DELEGATE(SumToCounter, int);
    SumToCounter delegate = SumToCounter::create([&](int i) { m_counter += i; });

    delegate.invoke(5);
    ASSERT_EQ(m_counter, 5);
}

TEST_F(TestDelegates, TestDelegateWithReturnType)
{
    DEFINE_DELEGATE_RET(Return10, int);
    Return10 delegate = Return10::create([&]() { return 10; });

    ASSERT_EQ(delegate.invoke(), 10);
}

TEST_F(TestDelegates, TestDelegateWithReturnTypeAndArguments)
{
    DEFINE_DELEGATE_RET(SumTo10, int, int);
    SumTo10 delegate = SumTo10::create([&](int i) { return 10 + i; });

    ASSERT_EQ(delegate.invoke(5), 15);
}