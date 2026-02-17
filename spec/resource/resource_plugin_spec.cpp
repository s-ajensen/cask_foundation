#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/resource/resource_store.hpp>
#include <cask/resource/mesh_data.hpp>
#include <cask/resource/texture_data.hpp>
#include <cstring>

struct ResourceTestContext : PluginTestContext {
    ResourceStore<MeshData>* mesh_store() {
        uint32_t store_id = world.register_component("MeshStore");
        return world.get<ResourceStore<MeshData>>(store_id);
    }

    ResourceStore<TextureData>* texture_store() {
        uint32_t store_id = world.register_component("TextureStore");
        return world.get<ResourceStore<TextureData>>(store_id);
    }
};

SCENARIO("resource plugin reports its metadata", "[resource]") {
    GIVEN("the resource plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is resource") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "resource") == 0);
        }

        THEN("it defines the MeshStore and TextureStore components") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "MeshStore") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "TextureStore") == 0);
        }

        THEN("it requires no components") {
            REQUIRE(info->requires_count == 0);
            REQUIRE(info->requires_components == nullptr);
        }

        THEN("it provides init and shutdown functions") {
            REQUIRE(info->init_fn != nullptr);
            REQUIRE(info->shutdown_fn != nullptr);
        }

        THEN("it does not provide tick or frame functions") {
            REQUIRE(info->tick_fn == nullptr);
            REQUIRE(info->frame_fn == nullptr);
        }
    }
}

SCENARIO("resource plugin initializes both stores", "[resource]") {
    GIVEN("a world and the resource plugin") {
        ResourceTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("MeshStore is registered and retrievable") {
                REQUIRE(context.mesh_store() != nullptr);
            }

            THEN("TextureStore is registered and retrievable") {
                REQUIRE(context.texture_store() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("resource plugin shutdown allows reinit on fresh world", "[resource]") {
    GIVEN("an initialized resource plugin") {
        ResourceTestContext context;
        context.init();

        REQUIRE(context.mesh_store() != nullptr);
        REQUIRE(context.texture_store() != nullptr);

        WHEN("shutdown is called and a fresh world is reinitialized") {
            context.shutdown();

            ResourceTestContext fresh_context;
            fresh_context.init();

            THEN("MeshStore is available on the fresh world") {
                REQUIRE(fresh_context.mesh_store() != nullptr);
            }

            THEN("TextureStore is available on the fresh world") {
                REQUIRE(fresh_context.texture_store() != nullptr);
            }

            fresh_context.shutdown();
        }
    }
}
