#pragma once 

#include "Archetypes.h"
#include "Types.h"
#include "Entity.h"

namespace ecs
{
	template<typename FirstComponent, typename... OtherComponents>
	struct query
	{
	public:
		query() {}
	};
}