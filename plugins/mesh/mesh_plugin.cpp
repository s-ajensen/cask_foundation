#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/mesh_data.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_compactor.hpp>

static ResourceStore<MeshData>* mesh_store = nullptr;
static ComponentStore<MeshHandle>* mesh_components = nullptr;

static void mesh_init(WorldHandle handle) {
    cask::WorldView world(handle);

    auto* compactor = world.get<EntityCompactor>(world.register_component("EntityCompactor"));

    mesh_store = new ResourceStore<MeshData>();
    uint32_t store_id = world.register_component("MeshStore");
    world.bind(store_id, mesh_store);

    mesh_components = new ComponentStore<MeshHandle>();
    uint32_t components_id = world.register_component("MeshComponents");
    world.bind(components_id, mesh_components);

    compactor->add(mesh_components, remove_component<MeshHandle>);
}

static void mesh_shutdown(WorldHandle) {
    delete mesh_store;
    mesh_store = nullptr;
    delete mesh_components;
    mesh_components = nullptr;
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
    mesh_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
