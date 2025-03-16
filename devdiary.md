# Dev Diary

## March 16, 2025

### Known issues

Implementation of ArchetypeDatabase and packed arrays of components are good enough. Optimization is needed in these aspects:
- computation of archetypes' hash codes. Replacing the archetype hash with a unique ID (like Components) might be better.
- double indexing of entities and components: to ensure contiguous packing of a component array, a map from entity to index and from index to entity because removed components need to be replaced by the component at the end of the array.
- still unsure about how to represent an archetype. Hopefully things will get more clear during the implementation of the query system


### Next steps

Next step is implementing an archetype query system: such system should allow to retrieve all the entities with a specific set of components. This might require rethinking about how the Archetypes Database is structured: for example, storing archetypes in a graph might be better for query optimization. **This is the only missing feature that can connect systems with entities and components**. The most straight-forward approach should be keeping an unordered_map that pairs each component_id with a set of all the archetypes that contain that vector. A query would pick the intersection of all these sets: the result is the set of all the archetypes we are looking for.

Another step could be to remove static methods and implement everything inside an ECS world container, to ensure proper data clean-up and consistency.