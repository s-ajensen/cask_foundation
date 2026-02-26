#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/identity/entity_registry.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/event/event_queue.hpp>

struct IdentityPluginState {
    EntityRegistry* registry;
    EventQueue<DestroyEntity>* destroy_queue;
};

static void identity_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* state = world.register_component<IdentityPluginState>("IdentityPluginState");
    state->registry = world.register_component<EntityRegistry>("EntityRegistry");
    state->destroy_queue = world.resolve<EventQueue<DestroyEntity>>("DestroyEntityQueue");
}

static void identity_tick(WorldHandle handle) {
    auto* state = static_cast<IdentityPluginState*>(world_resolve_component(handle, "IdentityPluginState"));
    if (!state || !state->registry || !state->destroy_queue) return;
    for (auto& event : state->destroy_queue->poll()) {
        state->registry->remove(event.entity);
    }
}

static const char* defined_components[] = {"EntityRegistry", "IdentityPluginState"};
static const char* required_components[] = {"EntityTable", "DestroyEntityQueue", "EventSwapper"};

static PluginInfo plugin_info = {
    "identity",
    defined_components,
    required_components,
    2,
    3,
    identity_init,
    identity_tick,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
