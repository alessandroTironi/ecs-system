#include <gtest/gtest.h>
#include <bit>
#include "ComponentArray.h"
#include "Entity.h"

struct FloatComponent : public ecs::IComponent
{
    float m_float = 0.0f;
};

TEST(TestComponentArray, TestConstructors)
{
    ASSERT_NO_THROW(ecs::component_array_base c1Array);
    ASSERT_NO_THROW(ecs::component_array_base c2Array(typeid(FloatComponent).hash_code(),
        sizeof(FloatComponent), 10));
}

TEST(TestComponentArray, TestStaticConstructor)
{
    ASSERT_NO_THROW(ecs::component_array_base c3Array = ecs::component_array_base::make<FloatComponent>(10));
}

TEST(TestComponentArray, TestAddComponent)
{
    const ecs::entity_id e = 0;

    ecs::component_array_base c = ecs::component_array_base::make<FloatComponent>(10);
    ASSERT_NO_THROW(c.allocate_component(e));
}

TEST(TestComponentArray, TestGetUnexistingComponent)
{
    const ecs::entity_id e = 0;
    ecs::component_array_base c = ecs::component_array_base::make<FloatComponent>(10);
    ASSERT_THROW(c.get_byte_ptr(0), std::out_of_range);
}

TEST(TestComponentArray, TestGetComponent)
{
    const ecs::entity_id e = 0;
    ecs::component_array_base c = ecs::component_array_base::make<FloatComponent>(10);
    c.allocate_component(e);
    ASSERT_NO_THROW(c.get_byte_ptr(0));
    ASSERT_NO_THROW(reinterpret_cast<FloatComponent*>(c.get_byte_ptr(0)));
    ASSERT_NO_THROW(FloatComponent& f = *std::bit_cast<FloatComponent*>(c.get_byte_ptr(0)));
}

TEST(TestComponentArray, TestRemoveComponent)
{
    ecs::component_array_base cArray = ecs::component_array_base::make<FloatComponent>(10);
    const ecs::entity_id e = 0;
    ASSERT_FALSE(cArray.free_component(e));

    cArray.allocate_component(e);

    ASSERT_NO_THROW(cArray.free_component(e));
    ASSERT_FALSE(cArray.free_component(e));

    cArray.allocate_component(e);
    ASSERT_TRUE(cArray.free_component(e));
}

TEST(TestComponentArray, TestSize)
{
    ecs::component_array_base cArray(typeid(FloatComponent).hash_code(),
        sizeof(FloatComponent), 10);
    ASSERT_EQ(cArray.size(), 0);

    cArray.allocate_component(0);
    ASSERT_EQ(cArray.size(), 1);

    cArray.free_component(0);
    ASSERT_EQ(cArray.size(), 0);
}

TEST(TestComponentArray, TestTemplateConstructor)
{
    static auto callConstructors = []()
    {
        auto c1 = ecs::component_array<FloatComponent>();
        auto c2 = ecs::component_array<FloatComponent, 10>();
    };
    
    ASSERT_NO_THROW(callConstructors());
}

TEST(TestComponentArray, TestTemplateAdd)
{
    ecs::component_array<FloatComponent, 10> cArray;
    FloatComponent& f = cArray.add_component(0);
}

TEST(TestComponentArray, TestTemplateFind)
{
    ecs::component_array<FloatComponent, 10> cArray;
    FloatComponent f;
    ASSERT_FALSE(cArray.find(0, f));
    
    cArray.add_component(0);
    ASSERT_TRUE(cArray.find(0, f));
}

TEST(TestComponentArray, TestTemplateGet)
{
    ecs::component_array<FloatComponent, 10> cArray;
    ASSERT_THROW(cArray.get(0), std::out_of_range);

    cArray.add_component(0);
    ASSERT_NO_THROW(FloatComponent& f = cArray.get(0));
}

TEST(TestComponentArray, TestTemplateRemove)
{
    ecs::component_array<FloatComponent, 10> cArray;
    ASSERT_FALSE(cArray.remove_component(0));

    cArray.add_component(0);
    ASSERT_TRUE(cArray.remove_component(0));
}