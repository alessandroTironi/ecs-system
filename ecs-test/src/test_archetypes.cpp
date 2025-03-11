#include <gtest/gtest.h>
#include <stdexcept>
#include "Archetypes.h"
#include "ComponentArray.h"

using ::testing::Test;

class TestArchetypes : public Test 
{
public:
    struct FloatComponent : public ecs::IComponent 
    {
    public:
        FloatComponent() {}
        FloatComponent(float value) : m_value(value) {}

        float m_value = 0.0f;
    };

    struct DoubleComponent : public ecs::IComponent 
    {
    public:
        DoubleComponent() {}
        DoubleComponent(double value) : m_value(value) {}

        double m_value = 0.0;
    };

    struct IntComponent : public ecs::IComponent 
    {
    public:
        IntComponent() {}
        IntComponent(int value) : m_value(value) {}

        int m_value = 0;
    };

protected:
    void SetUp() override
    {
        m_emptyArchetype = ecs::archetype();
    }

    void TearDown() override
    {
    }

    ecs::archetype m_emptyArchetype;
};

TEST_F(TestArchetypes, TestNullArchetype)
{
    EXPECT_TRUE(m_emptyArchetype.is_null()) << "An empty archetype should always be null";
}

TEST_F(TestArchetypes, TestEmptyArchetype)
{
    ASSERT_THROW(ecs::archetype::make({}), std::invalid_argument) <<   "It should not be possible to create an empty archetype: "
                                                                            "An invalid_argument exception should be thrown";
    ASSERT_NO_THROW(ecs::archetype::make({0}));
}

TEST_F(TestArchetypes, TestVectorMoveConstructor)
{
    std::vector<ecs::component_id> components = {};
    ASSERT_THROW(ecs::archetype(std::move(components)), std::invalid_argument) <<   "It should not be possible to create an empty archetype: "
                                                                                    "An invalid_argument exception should be thrown";
    std::vector<ecs::component_id> components2 = { 0 };
    ASSERT_NO_THROW(ecs::archetype(components2));
}

TEST_F(TestArchetypes, TestTemplateMakeFunction)
{
    ASSERT_NO_THROW(ecs::archetype::make<FloatComponent>());
    static constexpr auto createTwoComponents = []() { ecs::archetype::make<FloatComponent, IntComponent>(); };
    ASSERT_NO_THROW(createTwoComponents());
}