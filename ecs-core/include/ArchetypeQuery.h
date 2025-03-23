#pragma once 

#include <functional>
#include "Archetypes.h"
#include "ArchetypesRegistry.h"
#include "Types.h"
#include "World.h"
#include "Entity.h"

namespace ecs
{
	template<typename... Components>
	struct query
	{
	public:
		/** Type of the function that can be passed to the forEach() method. */
		using iteration_function = std::function<void(EntityHandle, Components&...)>;

		query() {}
		query(std::weak_ptr<World> world) : m_world(world) {}

		/**
		 * @brief Iterate over all entities that match the query and call the given function for each of them.
		 * @param func The function to call for each entity that matches the query.
		 * @note The actual query is performed here. Just building the query object doesn't perform any query.
		 */
		void forEach(iteration_function&& func)
		{
			
		}

	private:
		std::weak_ptr<World> m_world;
	};
}