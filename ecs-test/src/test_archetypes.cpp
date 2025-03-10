#include <gtest/gtest.h>
#include <stdexcept>
#include "Archetypes.h"

using ::testing::Test;

class TestArchetypes : public Test 
{

};

TEST_F(TestArchetypes, TestEmptyArchetype)
{
    ASSERT_THROW(auto empty = ecs::archetype(), std::invalid_argument) <<   "It should not be possible to create an empty archetype: "
                                                                            "An invalid_argument exception should be thrown";
}