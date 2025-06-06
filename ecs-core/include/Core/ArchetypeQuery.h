#pragma once 

#include <functional>
#include "Archetypes.h"
#include "ArchetypesRegistry.h"
#include "ComponentsRegistry.h"
#include "Types.h"
#include "World.h"
#include "Entity.h"
#include "QueryTypes.h"

namespace ecs
{
	template<typename... Components>
	struct query : public query_base
	{
	public:
		/** Type of the function that can be passed to the forEach() method. */
		using iteration_function = std::function<void(EntityHandle, Components&...)>;

		query() : query_base() {}
		query(std::weak_ptr<World> world) : query_base(world) {}

		/**
		 * @brief Iterate over all entities that match the query and call the given function for each of them.
		 * @param func The function to call for each entity that matches the query.
		 * @note The actual query is performed here. Just building the query object doesn't perform any query.
		 */
		void forEach(iteration_function&& func)
		{
			if (m_world.expired())
			{
				throw std::runtime_error("Attempt to make a query with an invalid world.");
			}

			if (ArchetypesRegistry* archetypesRegistry = m_world.lock()->GetArchetypesRegistry().get())
			{
				if (ComponentsRegistry* componentsRegistry = m_world.lock()->GetComponentsRegistry().get())
				{
					archetypesRegistry->ForEachEntity(func);
				}
			}
		}

		/**
		 * @brief Construct a query struct for the given world.
		 * @param world The world to make the query for.
		 * @return The query.
		 */
		static query<Components...> MakeQuery(std::weak_ptr<World> world) noexcept
		{
			return query<Components...>(world);
		}
	};
}