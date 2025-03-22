#pragma once 

#include <array>
#include <unordered_map>
#include <vector>
#include <limits>
#include "CompactString.h"

namespace ecs
{
    typedef float real_t;

    typedef size_t entity_id;
    const static entity_id INVALID_ENTITY_ID = std::numeric_limits<entity_id>::max();

    typedef unsigned int archetype_id;

    #define GetTypeHash(obj) typeid(obj).hash_code()

    typedef compact_string<40> name;
}