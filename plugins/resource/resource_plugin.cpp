#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/mesh_data.hpp>
#include <cask/resource/texture_data.hpp>

static ResourceStore<MeshData>* mesh_store = nullptr;
static ResourceStore<TextureData>* texture_store = nullptr;

static void resource_init(WorldHandle handle) {
    cask::WorldView world(handle);
    mesh_store = new ResourceStore<MeshData>();
    uint32_t mesh_id = world.register_component("MeshStore");
    world.bind(mesh_id, mesh_store);

    texture_store = new ResourceStore<TextureData>();
    uint32_t texture_id = world.register_component("TextureStore");
    world.bind(texture_id, texture_store);
}

static void resource_shutdown(WorldHandle) {
    delete mesh_store;
    mesh_store = nullptr;
    delete texture_store;
    texture_store = nullptr;
}

static const char* defined_components[] = {"MeshStore", "TextureStore"};

static PluginInfo plugin_info = {
    "resource",
    defined_components,
    nullptr,
    2,
    0,
    resource_init,
    nullptr,
    nullptr,
    resource_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
