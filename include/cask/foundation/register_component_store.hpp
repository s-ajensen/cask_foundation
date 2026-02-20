#pragma once

#include <cask/world.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_compactor.hpp>

namespace cask {

template<typename Component>
ComponentStore<Component>* register_component_store(WorldView& world, const char* name) {
    auto* store = world.register_component<ComponentStore<Component>>(name);
    auto* compactor = world.resolve<EntityCompactor>("EntityCompactor");
    compactor->add(store, remove_component<Component>);
    return store;
}

}
