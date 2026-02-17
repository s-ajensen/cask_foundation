#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/resource/resource_store.hpp>
#include <cask/resource/texture_data.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/event/event_queue.hpp>
#include <cstring>

struct TextureTestContext : CompactableTestContext {
    ResourceStore<TextureData>* texture_store() {
        uint32_t store_id = world.register_component("TextureStore");
        return world.get<ResourceStore<TextureData>>(store_id);
    }

    ComponentStore<TextureHandle>* texture_components() {
        uint32_t components_id = world.register_component("TextureComponents");
        return world.get<ComponentStore<TextureHandle>>(components_id);
    }
};

SCENARIO("texture plugin reports its metadata", "[texture]") {
    GIVEN("the texture plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is texture") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "texture") == 0);
        }

        THEN("it defines TextureStore and TextureComponents") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "TextureStore") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "TextureComponents") == 0);
        }

        THEN("it requires EntityCompactor") {
            REQUIRE(info->requires_count == 1);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EntityCompactor") == 0);
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

SCENARIO("texture plugin initializes TextureStore and TextureComponents", "[texture]") {
    GIVEN("a world with EntityCompactor and the texture plugin") {
        TextureTestContext context;
        REQUIRE(context.info->init_fn != nullptr);
        REQUIRE(context.info->shutdown_fn != nullptr);

        WHEN("init is called") {
            context.init();

            THEN("TextureStore is registered and retrievable") {
                REQUIRE(context.texture_store() != nullptr);
            }

            THEN("TextureComponents is registered and retrievable") {
                REQUIRE(context.texture_components() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("texture plugin wires TextureComponents into EntityCompactor", "[texture]") {
    GIVEN("an initialized texture plugin") {
        TextureTestContext context;
        REQUIRE(context.info->init_fn != nullptr);
        REQUIRE(context.info->shutdown_fn != nullptr);
        context.init();

        WHEN("an entity with a texture handle is compacted") {
            uint32_t entity = context.table.create();
            context.texture_components()->insert(entity, TextureHandle{0});

            EventQueue<DestroyEvent> destroy_queue;
            destroy_queue.emit(DestroyEvent{entity});
            destroy_queue.swap();
            context.compactor.compact(destroy_queue);

            THEN("the entity is no longer alive") {
                REQUIRE_FALSE(context.table.alive(entity));
            }

            THEN("the texture component is removed") {
                auto* components = context.texture_components();
                REQUIRE(components->entity_to_index_.find(entity) == components->entity_to_index_.end());
            }
        }

        context.shutdown();
    }
}

SCENARIO("texture plugin shutdown allows reinit on fresh world", "[texture]") {
    GIVEN("an initialized texture plugin") {
        TextureTestContext context;
        REQUIRE(context.info->init_fn != nullptr);
        REQUIRE(context.info->shutdown_fn != nullptr);
        context.init();

        REQUIRE(context.texture_store() != nullptr);
        REQUIRE(context.texture_components() != nullptr);

        WHEN("shutdown is called and a fresh world is reinitialized") {
            context.shutdown();

            TextureTestContext fresh_context;
            fresh_context.init();

            THEN("TextureStore is available on the fresh world") {
                REQUIRE(fresh_context.texture_store() != nullptr);
            }

            THEN("TextureComponents is available on the fresh world") {
                REQUIRE(fresh_context.texture_components() != nullptr);
            }

            fresh_context.shutdown();
        }
    }
}
