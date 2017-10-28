/**
 * Created by bwiberg on 9/11/17.
 */

#pragma once

#include "EcsTypes.hh"
#include "internal/InternalEcsTypes.hh"
#include "ComponentFilter.hh"

namespace cfl {
namespace ecs {
using namespace internal;

class System {
    friend class EcsManager;

    EcsManager * manager;

    component_bitmask_t const componentBitmask;

    auto FindSystemIndex() const;

protected:
    System(component_filter_t const &component_filter);

    /**
     * Pure virtual destructor to make class abstract.
     */
    virtual ~System() = 0;

    virtual void UpdateEntity(Entity &entity) = 0;

public:
    bool IsValid() const;

    void Enable();

    void Disable();

    void RemoveSelf();
};

template<typename ...RequiredComponents>
struct BaseSystem : public System {
    BaseSystem();
};

inline System::System(component_filter_t const &component_filter) : componentBitmask(component_filter) {}

inline System::~System() {}

template<typename ...RequiredComponents>
BaseSystem<RequiredComponents...>::BaseSystem() : System(ComponentFilter<RequiredComponents...>::Get()) {}
} // namespace ecs
} // namespace cfl
