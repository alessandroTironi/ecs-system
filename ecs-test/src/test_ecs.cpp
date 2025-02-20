#include <gtest/gtest.h>
#include "ECSInstance.h"
#include "ISystem.h"

class MockSystem : public ecs::ISystem
{
    void Update(ecs::real_t deltaTime) override {}
};

TEST(TestECSInstance, TestConstructor)
{
    ecs::Instance* instance = nullptr;
    ASSERT_NO_THROW(instance = new ecs::Instance());
    delete instance;
}

TEST(TestECSInstance, TestInitialize)
{
    ecs::Instance* instance = new ecs::Instance();
    ASSERT_NO_THROW(instance->Initialize());

    delete instance;
}

TEST(TestECSInstance, TestAddSystems)
{
    std::shared_ptr<MockSystem> mockSystem = std::shared_ptr<MockSystem>(new MockSystem());
    ecs::Instance instance = ecs::Instance();
    ASSERT_EQ(instance.GetSystemsCount(), 0);

    ASSERT_NO_THROW(instance.AddSystem(mockSystem));
    ASSERT_NO_THROW(instance.AddSystem(mockSystem));
    ASSERT_EQ(instance.GetSystemsCount(), 1);
}

TEST(TestECSInstance, TestRemoveSystems)
{
    std::shared_ptr<MockSystem> mockSystem = std::shared_ptr<MockSystem>(new MockSystem());
    ecs::Instance instance = ecs::Instance();
    ASSERT_EQ(instance.GetSystemsCount(), 0);

    instance.AddSystem(mockSystem);
    ASSERT_EQ(instance.GetSystemsCount(), 1);

    instance.RemoveSystem(mockSystem);
    ASSERT_EQ(instance.GetSystemsCount(), 0);

    ASSERT_NO_THROW(instance.RemoveSystem(mockSystem));
}

TEST(TestECSInstance, TestUpdate)
{
    static constexpr float frameTime30 = 1.0f / 30.0f;
    ecs::Instance* instance = new ecs::Instance();
    ASSERT_NO_THROW(instance->Update(frameTime30));

    std::shared_ptr<MockSystem> mockSystem = std::shared_ptr<MockSystem>(new MockSystem());
    instance->AddSystem(mockSystem);

    ASSERT_NO_THROW(instance->Update(frameTime30));

    delete instance;
}