#pragma once 

#include <array>
#include <unordered_map>
#include <vector>
#include "CompactString.h"

namespace ecs
{
    typedef float real_t;

    typedef unsigned int archetype_id;

    #define GetTypeHash(obj) typeid(obj).hash_code()

    typedef compact_string<40> name;
}