#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/ecs/frame_advancer.hpp>

struct InterpolationPluginState {
    FrameAdvancer* advancer;
};

static void interpolation_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* state = world.register_component<InterpolationPluginState>("InterpolationPluginState");
    state->advancer = world.register_component<FrameAdvancer>("FrameAdvancer");
}

static void interpolation_tick(WorldHandle handle) {
    auto* state = static_cast<InterpolationPluginState*>(world_resolve_component(handle, "InterpolationPluginState"));
    if (!state || !state->advancer) return;
    state->advancer->advance_all();
}

static const char* defined_components[] = {"FrameAdvancer", "InterpolationPluginState"};

static PluginInfo plugin_info = {
    "interpolation",
    defined_components,
    nullptr,
    2,
    0,
    interpolation_init,
    interpolation_tick,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
