#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/identity/entity_registry.hpp>
#include <cask/identity/uuid.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/event/event_swapper.hpp>
#include <cask/event/event_queue.hpp>
#include <cstring>

struct IdentityTestContext : PluginTestContext {
    EntityTable table;
    EventSwapper swapper;
    EventQueue<DestroyEntity> destroy_queue;

    IdentityTestContext() {
        uint32_t table_id = world.register_component("EntityTable");
        world.bind(table_id, &table);
        uint32_t queue_id = world.register_component("DestroyEntityQueue");
        world.bind(queue_id, &destroy_queue);
        uint32_t swapper_id = world.register_component("EventSwapper");
        world.bind(swapper_id, &swapper);
        swapper.add(&destroy_queue, swap_queue<DestroyEntity>);
    }

    EntityRegistry* registry() {
        return static_cast<EntityRegistry*>(world.resolve("EntityRegistry"));
    }
};

SCENARIO("identity plugin reports its metadata", "[identity]") {
    GIVEN("the identity plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is identity") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "identity") == 0);
        }

        THEN("it defines EntityRegistry and IdentityPluginState") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "EntityRegistry") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "IdentityPluginState") == 0);
        }

        THEN("it requires EntityTable DestroyEntityQueue and EventSwapper") {
            REQUIRE(info->requires_count == 3);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EntityTable") == 0);
            REQUIRE(std::strcmp(info->requires_components[1], "DestroyEntityQueue") == 0);
            REQUIRE(std::strcmp(info->requires_components[2], "EventSwapper") == 0);
        }

        THEN("it provides init and tick functions") {
            REQUIRE(info->init_fn != nullptr);
            REQUIRE(info->tick_fn != nullptr);
        }

        THEN("it does not provide a shutdown function") {
            REQUIRE(info->shutdown_fn == nullptr);
        }

        THEN("it does not provide a frame function") {
            REQUIRE(info->frame_fn == nullptr);
        }
    }
}

SCENARIO("identity plugin initializes EntityRegistry", "[identity]") {
    GIVEN("a world with required components") {
        IdentityTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("EntityRegistry is registered and retrievable") {
                REQUIRE(context.registry() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("identity plugin removes identity on entity destroy", "[identity]") {
    GIVEN("an initialized identity plugin with a registered entity") {
        IdentityTestContext context;
        context.init();

        auto* registry = context.registry();
        REQUIRE(registry != nullptr);
        cask::UUID uuid = cask::generate_uuid();
        uint32_t entity = registry->resolve(uuid, context.table);
        REQUIRE(registry->has(entity));

        WHEN("a DestroyEntity event is emitted and tick is called") {
            context.destroy_queue.emit(DestroyEntity{entity});
            context.swapper.swap_all();
            context.tick();

            THEN("the entity identity is removed from the registry") {
                REQUIRE_FALSE(registry->has(entity));
            }
        }

        context.shutdown();
    }
}

SCENARIO("identity plugin ignores destroy for entity without uuid", "[identity]") {
    GIVEN("an initialized identity plugin with an entity that has no uuid") {
        IdentityTestContext context;
        context.init();

        auto* registry = context.registry();
        REQUIRE(registry != nullptr);
        uint32_t entity = context.table.create();
        REQUIRE_FALSE(registry->has(entity));
        size_t size_before = registry->size();

        WHEN("a DestroyEntity event is emitted and tick is called") {
            context.destroy_queue.emit(DestroyEntity{entity});
            context.swapper.swap_all();
            context.tick();

            THEN("the registry is unchanged") {
                REQUIRE(registry->size() == size_before);
            }
        }

        context.shutdown();
    }
}

SCENARIO("identity plugin isolates state across worlds", "[identity]") {
    GIVEN("two initialized worlds each with a registered entity") {
        IdentityTestContext context1;
        IdentityTestContext context2;
        context1.init();
        context2.init();

        auto* registry1 = context1.registry();
        auto* registry2 = context2.registry();
        REQUIRE(registry1 != nullptr);
        REQUIRE(registry2 != nullptr);

        cask::UUID uuid1 = cask::generate_uuid();
        uint32_t entity1 = registry1->resolve(uuid1, context1.table);
        REQUIRE(registry1->has(entity1));

        cask::UUID uuid2 = cask::generate_uuid();
        uint32_t entity2 = registry2->resolve(uuid2, context2.table);
        REQUIRE(registry2->has(entity2));

        WHEN("a destroy event is emitted only on world1") {
            context1.destroy_queue.emit(DestroyEntity{entity1});
            context1.swapper.swap_all();
            context1.tick();

            THEN("world1 entity identity is removed") {
                REQUIRE_FALSE(registry1->has(entity1));
            }

            THEN("world2 entity identity is still present") {
                REQUIRE(registry2->has(entity2));
            }
        }

        context2.shutdown();
        context1.shutdown();
    }
}
