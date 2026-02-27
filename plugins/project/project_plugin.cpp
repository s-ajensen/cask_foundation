#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/project_root.hpp>
#include <cask/platform/executable_path.hpp>
#include <cstdlib>

static void project_init(WorldHandle handle) {
    cask::WorldView world(handle);

    const char* env_value = std::getenv("CASK_PROJECT_ROOT");
    bool has_env = (env_value != nullptr && env_value[0] != '\0');

    auto* existing = static_cast<ProjectRoot*>(world_resolve_component(handle, "ProjectRoot"));

    if (has_env && existing) {
        existing->path = env_value;
        return;
    }
    if (has_env) {
        auto* root = world.register_component<ProjectRoot>("ProjectRoot");
        root->path = env_value;
        return;
    }
    if (existing) {
        return;
    }
    auto* root = world.register_component<ProjectRoot>("ProjectRoot");
    root->path = cask::executable_directory();
}

static const char* defined_components[] = {"ProjectRoot"};

static PluginInfo plugin_info = {
    "project",
    defined_components,
    nullptr,
    1,
    0,
    project_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
