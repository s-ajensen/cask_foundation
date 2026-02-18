#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/mesh_data.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_compactor.hpp>

static void mesh_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* compactor = world.resolve<EntityCompactor>("EntityCompactor");
    world.register_component<ResourceStore<MeshData>>("MeshStore");
    auto* mesh_components = world.register_component<ComponentStore<MeshHandle>>("MeshComponents");
    compactor->add(mesh_components, remove_component<MeshHandle>);
}

static const char* defined_components[] = {"MeshStore", "MeshComponents"};
static const char* required_components[] = {"EntityCompactor"};

static PluginInfo plugin_info = {
    "mesh",
    defined_components,
    required_components,
    2,
    1,
    mesh_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
