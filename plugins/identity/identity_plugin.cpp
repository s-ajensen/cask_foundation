#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/identity/entity_registry.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/event/event_queue.hpp>

static EntityRegistry* registry = nullptr;
static EventQueue<DestroyEntity>* destroy_queue = nullptr;

static void identity_init(WorldHandle handle) {
    cask::WorldView world(handle);
    registry = world.register_component<EntityRegistry>("EntityRegistry");
    destroy_queue = world.resolve<EventQueue<DestroyEntity>>("DestroyEntityQueue");
}

static void identity_tick(WorldHandle) {
    if (!registry || !destroy_queue) return;
    for (auto& event : destroy_queue->poll()) {
        registry->remove(event.entity);
    }
}

static void identity_shutdown(WorldHandle) {
    registry = nullptr;
    destroy_queue = nullptr;
}

static const char* defined_components[] = {"EntityRegistry"};
static const char* required_components[] = {"EntityTable", "DestroyEntityQueue", "EventSwapper"};

static PluginInfo plugin_info = {
    "identity",
    defined_components,
    required_components,
    1,
    3,
    identity_init,
    identity_tick,
    nullptr,
    identity_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
