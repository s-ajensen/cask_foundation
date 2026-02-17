#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/event/event_swapper.hpp>

static EventSwapper* swapper = nullptr;

static void event_init(WorldHandle handle) {
    cask::WorldView world(handle);
    swapper = new EventSwapper();
    uint32_t swapper_id = world.register_component("EventSwapper");
    world.bind(swapper_id, swapper);
}

static void event_tick(WorldHandle) {
    if (!swapper) return;
    swapper->swap_all();
}

static void event_shutdown(WorldHandle) {
    delete swapper;
    swapper = nullptr;
}

static const char* defined_components[] = {"EventSwapper"};

static PluginInfo plugin_info = {
    "event",
    defined_components,
    nullptr,
    1,
    0,
    event_init,
    event_tick,
    nullptr,
    event_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
