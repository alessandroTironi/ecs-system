# Dev Diary

## March 16, 2025

### Known issues

Implementation of ArchetypeDatabase and packed arrays of components are good enough. Optimization is needed in these aspects:
- computation of archetypes' hash codes. Replacing the archetype hash with a unique ID (like Components) might be better.
- double indexing of entities and components: to ensure contiguous packing of a component array, a map from entity to index and from index to entity because removed components need to be replaced by the component at the end of the array.
- still unsure about how to represent an archetype. Hopefully things will get more clear during the implementation of the query system

**Important**: using the returned value of typeinfo.hash() as a hash code for components can cause issues, since such hash code is not guaranteed to be unique. We need to switch to strings, unfortunately... but maybe we can add some constraint to have them stored inline (e.g., a limit on the amount of characters).


### Next steps

Next step is implementing an archetype query system: such system should allow to retrieve all the entities with a specific set of components. This might require rethinking about how the Archetypes Database is structured: for example, storing archetypes in a graph might be better for query optimization. **This is the only missing feature that can connect systems with entities and components**. The most straight-forward approach should be keeping an unordered_map that pairs each component_id with a set of all the archetypes that contain that vector. A query would pick the intersection of all these sets: the result is the set of all the archetypes we are looking for.

Another step could be to remove static methods and implement everything inside an ECS world container, to ensure proper data clean-up and consistency.



## March 19, 2025

Differently from what stated in the last entry, a major priority has been given to switching to component_id and names as keys for unordered_maps, to avoid making the next implementations more complicated. ComponentsDatabase and ArchetypesDatabase are now non-static classes, and ArchetypesDatabase has a member variable of type ComponentsDatabase. In the future, the ecs::World class will handle all the passing of references between the major components of the entire system.

The entity_t class is going towards deprecation. At this point, entities could basically only exist as IDs in the ArchetypesDatabase. A good idea for a better quality of life could be implementing an entity_handle_t class that contains the entity ID and a reference to the ecs::World class. An entity_handle_t can be queried to the ArchetypsDatabase, which would generate it dynamically. This could allow to have an easy-to-use interface to entities and to avoid storing the same pointer to ecs::World on all entities. 

### Next steps 

Implementing archetype queries remains the next big step, but first, we should make sure that hash values are not used as keys anymore. In particular, archetypes also need to have a unique serial ID.



## March 23, 2025

The EntityHandle class is implemented and we now have a public interface to the entire system with decent QoL methods. The class ECSInstance is going to be deprecated as soon as the Systems registration and execution part is moved to the World class, which is going to replace it. 

It's finally time to build a query system. After that, we need to move the systems functionality to World and we should be finally ready to test the complete ECS system for the first time.

### Known issues

The entire handling of the archetypes could be better. I don't have a clear understanding of how to clean things up yet, but I have the feeling that the Archetypes Registry could perform less lookups and avoid some memory allocations. It would be nice to store the component IDs of an archetype in a plain array to avoid memory allocations, but that would require to impose a limit to the amount of components that can be attached to an entity, which is unlikely to happen.

### Next steps

Besides building the query system, a couple of interesting ideas for reducing memory allocations could be:
- writing custom memory allocators for unordered maps
- in archetypes, store the first 8/16 components inline and the others on the heap.


## March 25, 2025

The first loop of the complete system is finally finished. I tested it with a basic SDL app that spawn 20k dots and makes them bounce inside the window. The frame rate was 30fps: I must say that I expected some more, but it's also true that the entire system can still be heavily optimized, so that's what I will focus on in the next days, until I reach a point which is satifying enough to be named version 1.0. I will start collecting a todo list in each entry of this diary, and keep it updated during development.

### TO DO list
- Replace the use of ecs::name as key type for maps with std::type_index, which does not allocate memory and it's much more reliable.
- Improve the iteration over entities of a query: it still feels it can be improved.
- Consider using partially inline data structs, i.e., data struct with a default size for inline data and a pointer to dynamically allocated memory that stores data exceeding the default inline size.
- Implemented cached queries, i.e., queries that store the result (in terms of set of archetypes) so that it does not need to be recomputed in the future.