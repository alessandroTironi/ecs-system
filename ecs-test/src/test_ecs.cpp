#include <gtest/gtest.h>
#include "ECSInstance.h"

TEST(TestECSInstance, TestConstructor)
{
    ecs::ECSInstance* instance = nullptr;
    ASSERT_NO_THROW(instance = new ecs::ECSInstance());
    delete instance;
}

TEST(TestECSInstance, TestInitialize)
{
    ecs::ECSInstance* instance = new ecs::ECSInstance();
    ASSERT_NO_THROW(instance->Initialize());

    delete instance;
}

TEST(TestECSInstance, TestUpdate)
{
    static constexpr float frameTime30 = 1.0f / 30.0f;
    ecs::ECSInstance* instance = new ecs::ECSInstance();
    ASSERT_NO_THROW(instance->Update(frameTime30));

    delete instance;
}