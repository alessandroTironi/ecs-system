#pragma once

#include <memory>
#include "Types.h"
#include "Entity.h"

namespace ecs 
{
	class ArchetypesRegistry; 
	class ComponentsRegistry;

	class World
	{
	public:
		World();
		~World();

	private:
		std::shared_ptr<ArchetypesRegistry> m_archetypesRegistry;
		std::shared_ptr<ComponentsRegistry> m_componentsRegistry;
	};
}