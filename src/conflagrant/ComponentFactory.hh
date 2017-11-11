#pragma once

#include <unordered_map>
#include <entityx/Entity.h>
#include <conflagrant/types.hh>
#include <conflagrant/serialization/Serialize.hh>

namespace cfl {
struct ComponentFactory {
    virtual bool Create(entityx::Entity &entity, Json::Value &json) const = 0;

    virtual bool HasComponent(entityx::Entity &entity) const = 0;

    virtual bool Serialize(Json::Value &json, entityx::Entity &entity) const = 0;
};

template<typename TComponent>
struct ConcreteComponentFactory : public ComponentFactory {
    bool Create(entityx::Entity &entity, Json::Value &json) const override {
        entityx::ComponentHandle<TComponent> component = entity.assign<TComponent>();
        return TComponent::template Serialize<Deserializer>(json, *component.get());
    }

    bool HasComponent(entityx::Entity &entity) const override {
        return entity.has_component<TComponent>();
    }

    bool Serialize(Json::Value &json, entityx::Entity &entity) const override {
        return TComponent::template Serialize<Serializer>(json, *entity.component<TComponent>().get());
    }
};

extern std::unordered_map<string, std::shared_ptr<ComponentFactory>> ComponentFactoriesByName;

#define REGISTER_COMPONENT(component_t) cfl::ComponentFactoriesByName[component_t::GetName()] = \
std::make_shared<cfl::ConcreteComponentFactory<component_t>>()

#define CLEAR_REGISTERED_COMPONENTS() cfl::ComponentFactoriesByName.clear()
} // namespace cfl