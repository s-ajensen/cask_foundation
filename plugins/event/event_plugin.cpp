#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/event/event_swapper.hpp>

struct EventPluginState {
    EventSwapper* swapper;
};

static void event_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* state = world.register_component<EventPluginState>("EventPluginState");
    state->swapper = world.register_component<EventSwapper>("EventSwapper");
}

static void event_tick(WorldHandle handle) {
    auto* state = static_cast<EventPluginState*>(world_resolve_component(handle, "EventPluginState"));
    if (!state || !state->swapper) return;
    state->swapper->swap_all();
}

static const char* defined_components[] = {"EventSwapper", "EventPluginState"};

static PluginInfo plugin_info = {
    "event",
    defined_components,
    nullptr,
    2,
    0,
    event_init,
    event_tick,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
