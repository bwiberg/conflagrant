/**
 * Created by bwiberg on 9/11/17.
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include "../EcsTypes.hh"
#include "InternalEcsTypes.hh"
#include "ComponentType.hh"
#include "EcsAssert.hh"

namespace cfl {
namespace ecs {
namespace internal {
enum class EntityState {
    Enabled,
    Disabled,
    Destroyed
};

struct EntityData {
    inline EntityData(entity_id_t id,
                      component_bitmask_t const &componentBitmask,
                      EntityState entity_state)
            : id(id),
              componentBitmask(componentBitmask),
              state(entity_state) {}

    static const EntityData Null;

    /**
     * The current lifecycle ID of this entity, i.e. how many times this handle
     * has been reused.
     */
    entity_id_t id;

    /**
     * A bitmask containing what components this entity has currently.
     */
    component_bitmask_t componentBitmask;

    EntityState state;
};

class EntityStorage {
    friend class ::cfl::ecs::EcsManager;

    friend class ::cfl::ecs::Entity;

    /**
     * Array (vector) that stores ALL components of ALL types of ALL entities.
     */
    MegaArray megaArray;

    std::vector<EntityData> entityData;

    /**
     * Array (vector) that stores the offset of each entity in the MegaArray.
     */
    std::vector<entity_offset_t> entityOffsets;

    /**
     * The offset of an entity's component of a given type for ALL entities
     * and ALL component types.
     */
    std::vector<std::vector<component_offset_t>> componentOffsetsByType;

    /**
     * Array (vector) that stores the next "lifetime" id of each entity.
     */
    std::vector<entity_id_t> nextEntityIds;

    entity_index_t currentMaxIndex;

    /**
     * MegaArray offset to the next newly created entity's components.
     */
    entity_offset_t nextEntityOffset;

    /**
     * Entity index of the next new entity.
     */
    entity_index_t nextEntityIndex;

    std::unordered_map<component_bitmask_t, std::queue<entity_index_t>> freeEntityIndicesByComponentBitmask;

    void ReserveTablesForEntityIndex(entity_index_t index);

    void ReuseReservedComponentMemory(Entity &e, std::queue<entity_index_t> &freeEntityIndices);

    void StoreEntityData(Entity const &entity);

public:
    EntityStorage(size_t initialMemory = CFL_DEFAULT_COMPONENTSTORAGE_MEMORY,
                  entity_index_t initial_entities_capacity = CFL_DEFAULT_INITIAL_ENTITIES_CAPACITY);

    void ReserveComponentMemory(Entity &entity,
                                component_bitmask_t const &component_bitmask);

    void ReserveComponentMemory(std::vector<Entity> &entities,
                                component_bitmask_t const &component_bitmask);

    void FreeEntityMemory(Entity &e);

    template<typename TComponent, typename ...Args>
    TComponent &InitializeComponent(entity_index_t entity_index, Args &&...args);

    template<typename TComponent>
    TComponent &GetComponent(entity_index_t entity_index);

    bool HasComponent(entity_index_t entity_index, component_id_t component_id);

    void RemoveComponent(entity_index_t entity_index, component_id_t component_id);
};

template<typename TComponent, typename ...Args>
TComponent &EntityStorage::InitializeComponent(entity_index_t entity_index, Args &&...args) {
    component_id_t component_id = ComponentType::TypeId<TComponent>();
    entityData[entity_index].componentBitmask.set(component_id, true);

    component_offset_t component_offset = componentOffsetsByType[component_id][entity_index];
    Assert(component_offset != storage::InvalidComponentOffset, "Tried to initialize unhinted component.");
    size_t component_address = storage::GetActualComponentOffset(entityOffsets[entity_index] + component_offset);

    TComponent *ptr = new(&megaArray[component_address]) TComponent{std::forward<Args>(args)...};
    return *ptr;
};

template<typename TComponent>
TComponent &EntityStorage::GetComponent(entity_index_t entity_index) {
    component_id_t component_id = ComponentType::TypeId<TComponent>();
    Assert(entityData[entity_index].componentBitmask.test(component_id),
           "Tried to get component that entity did not have.");

    component_offset_t component_offset = componentOffsetsByType[component_id][entity_index];
    Assert(component_offset != storage::InvalidComponentOffset,
           "Component offset is invalid even though bitmask is set.");
    size_t component_address = storage::GetActualComponentOffset(entityOffsets[entity_index] + component_offset);

    return *reinterpret_cast<TComponent *>(&megaArray[component_address]);
};

inline bool EntityStorage::HasComponent(entity_index_t entity_index, component_id_t component_id) {
    bool hasMemoryForComponent = componentOffsetsByType[component_id][entity_index] != storage::InvalidComponentOffset;
    bool componentIsActive = entityData[entity_index].componentBitmask.test(component_id);

    return hasMemoryForComponent && componentIsActive;
}

inline void EntityStorage::RemoveComponent(entity_index_t entity_index, component_id_t component_id) {
    Assert(HasComponent(entity_index, component_id), "Trying to remove component that entity does not have.");
    entityData[entity_index].componentBitmask.set(component_id, false);
}
} // namespace internal
} // namespace ecs
} // namespace cfl
