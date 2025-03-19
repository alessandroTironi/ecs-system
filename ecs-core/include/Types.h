#pragma once 

#include <array>
#include <unordered_map>
#include <vector>
#include "CompactString.h"

namespace ecs
{
    typedef float real_t;

    typedef size_t type_hash_t;

    #define GetTypeHash(obj) typeid(obj).hash_code()

    typedef compact_string<40> name;
}