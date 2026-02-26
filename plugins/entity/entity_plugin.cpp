#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/foundation/register_event_queue.hpp>

struct EntityPluginState {
    EntityCompactor* compactor;
    EventQueue<DestroyEntity>* destroy_queue;
};

static void entity_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* state = world.register_component<EntityPluginState>("EntityPluginState");
    auto* table = world.register_component<EntityTable>("EntityTable");
    state->compactor = world.register_component<EntityCompactor>("EntityCompactor");
    state->compactor->table_ = table;
    state->destroy_queue = cask::register_event_queue<DestroyEntity>(world, "DestroyEntityQueue");
}

static void entity_tick(WorldHandle handle) {
    auto* state = static_cast<EntityPluginState*>(world_resolve_component(handle, "EntityPluginState"));
    if (!state || !state->compactor || !state->destroy_queue) return;
    state->compactor->compact(*state->destroy_queue);
}

static const char* defined_components[] = {"EntityTable", "EntityCompactor", "DestroyEntityQueue", "EntityPluginState"};
static const char* required_components[] = {"EventSwapper"};

static PluginInfo plugin_info = {
    "entity",
    defined_components,
    required_components,
    4,
    1,
    entity_init,
    entity_tick,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
