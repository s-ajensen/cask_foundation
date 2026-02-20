#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/foundation/register_event_queue.hpp>

static EntityCompactor* compactor = nullptr;
static EventQueue<DestroyEntity>* destroy_queue = nullptr;

static void entity_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* table = world.register_component<EntityTable>("EntityTable");
    compactor = world.register_component<EntityCompactor>("EntityCompactor");
    compactor->table_ = table;
    destroy_queue = cask::register_event_queue<DestroyEntity>(world, "DestroyEntityQueue");
}

static void entity_tick(WorldHandle) {
    if (!compactor || !destroy_queue) return;
    compactor->compact(*destroy_queue);
}

static void entity_shutdown(WorldHandle) {
    compactor = nullptr;
    destroy_queue = nullptr;
}

static const char* defined_components[] = {"EntityTable", "EntityCompactor", "DestroyEntityQueue"};
static const char* required_components[] = {"EventSwapper"};

static PluginInfo plugin_info = {
    "entity",
    defined_components,
    required_components,
    3,
    1,
    entity_init,
    entity_tick,
    nullptr,
    entity_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
