#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cstring>

struct EntityTestContext : PluginTestContext {
    EntityTable* entity_table() {
        uint32_t table_id = world.register_component("EntityTable");
        return world.get<EntityTable>(table_id);
    }

    EntityCompactor* compactor() {
        uint32_t compactor_id = world.register_component("EntityCompactor");
        return world.get<EntityCompactor>(compactor_id);
    }
};

SCENARIO("entity plugin reports its metadata", "[entity]") {
    GIVEN("the entity plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is entity") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "entity") == 0);
        }

        THEN("it defines the EntityTable and EntityCompactor components") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "EntityTable") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "EntityCompactor") == 0);
        }

        THEN("it requires the EventSwapper component") {
            REQUIRE(info->requires_count == 1);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EventSwapper") == 0);
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

SCENARIO("entity plugin initializes EntityTable and EntityCompactor", "[entity]") {
    GIVEN("a world and the entity plugin") {
        EntityTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("EntityTable is registered and retrievable") {
                REQUIRE(context.entity_table() != nullptr);
            }

            THEN("EntityCompactor is registered and retrievable") {
                REQUIRE(context.compactor() != nullptr);
            }

            THEN("the compactor table pointer is set to the entity table") {
                REQUIRE(context.compactor()->table_ == context.entity_table());
            }

            context.shutdown();
        }
    }
}

SCENARIO("entity plugin shutdown allows reinit on fresh world", "[entity]") {
    GIVEN("an initialized entity plugin") {
        EntityTestContext context;
        context.init();

        REQUIRE(context.entity_table() != nullptr);
        REQUIRE(context.compactor() != nullptr);

        WHEN("shutdown is called and a fresh world is reinitialized") {
            context.shutdown();

            EntityTestContext fresh_context;
            fresh_context.init();

            THEN("EntityTable is available on the fresh world") {
                REQUIRE(fresh_context.entity_table() != nullptr);
            }

            THEN("EntityCompactor is available on the fresh world") {
                REQUIRE(fresh_context.compactor() != nullptr);
            }

            fresh_context.shutdown();
        }
    }
}
