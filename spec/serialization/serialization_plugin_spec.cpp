#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/schema/serialization_registry.hpp>
#include <cask/identity/entity_registry.hpp>
#include <cask/identity/uuid.hpp>
#include <nlohmann/json.hpp>
#include <cstring>

struct SerializationTestContext : PluginTestContext {
    EntityTable table;
    EntityRegistry* entity_registry;

    SerializationTestContext() {
        uint32_t table_id = world.register_component("EntityTable");
        world.bind(table_id, &table);

        entity_registry = new EntityRegistry();
        uint32_t registry_id = world.register_component("EntityRegistry");
        world.bind(registry_id, entity_registry);
    }

    ~SerializationTestContext() {
        delete entity_registry;
    }

    cask::SerializationRegistry* serialization_registry() {
        return static_cast<cask::SerializationRegistry*>(world.resolve("SerializationRegistry"));
    }
};

SCENARIO("serialization plugin reports its metadata", "[serialization]") {
    GIVEN("the serialization plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is serialization") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "serialization") == 0);
        }

        THEN("it defines SerializationRegistry and SerializationPluginState") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "SerializationRegistry") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "SerializationPluginState") == 0);
        }

        THEN("it requires EntityTable and EntityRegistry") {
            REQUIRE(info->requires_count == 2);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EntityTable") == 0);
            REQUIRE(std::strcmp(info->requires_components[1], "EntityRegistry") == 0);
        }

        THEN("it provides only an init function") {
            REQUIRE(info->init_fn != nullptr);
            REQUIRE(info->tick_fn == nullptr);
            REQUIRE(info->frame_fn == nullptr);
            REQUIRE(info->shutdown_fn == nullptr);
        }
    }
}

SCENARIO("serialization plugin initializes SerializationRegistry", "[serialization]") {
    GIVEN("a world with EntityTable and EntityRegistry") {
        SerializationTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("SerializationRegistry is registered and retrievable") {
                REQUIRE(context.serialization_registry() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("serialization plugin isolates state between worlds", "[serialization]") {
    GIVEN("two worlds each with the serialization plugin initialized") {
        SerializationTestContext world1;
        SerializationTestContext world2;
        world1.init();
        world2.init();

        auto* registry1 = world1.serialization_registry();
        auto* registry2 = world2.serialization_registry();

        THEN("each world has its own SerializationRegistry instance") {
            REQUIRE(registry1 != nullptr);
            REQUIRE(registry2 != nullptr);
            REQUIRE(registry1 != registry2);
        }

        THEN("each registry independently has the EntityRegistry serializer") {
            REQUIRE(registry1->has("EntityRegistry"));
            REQUIRE(registry2->has("EntityRegistry"));
        }

        world2.shutdown();
        world1.shutdown();
    }
}

SCENARIO("serialization plugin registers EntityRegistry serializer", "[serialization]") {
    GIVEN("an initialized serialization plugin") {
        SerializationTestContext context;
        context.init();

        WHEN("the SerializationRegistry is inspected") {
            auto* registry = context.serialization_registry();

            THEN("it contains an entry for EntityRegistry") {
                REQUIRE(registry->has("EntityRegistry"));
            }
        }

        context.shutdown();
    }
}

SCENARIO("EntityRegistry can be serialized through the registry", "[serialization]") {
    GIVEN("an initialized serialization plugin with entities in the registry") {
        SerializationTestContext context;
        context.init();

        auto* entity_registry = context.entity_registry;
        cask::UUID uuid = cask::generate_uuid();
        uint32_t entity = entity_registry->resolve(uuid, context.table);

        WHEN("the EntityRegistry is serialized via the SerializationRegistry") {
            auto* ser_registry = context.serialization_registry();
            auto& entry = ser_registry->get("EntityRegistry");
            auto json = entry.serialize(entity_registry);

            THEN("the JSON contains the entity mapped by UUID") {
                std::string uuid_str = uuids::to_string(uuid);
                REQUIRE(json.contains(uuid_str));
                REQUIRE(json[uuid_str].get<uint32_t>() == entity);
            }
        }

        context.shutdown();
    }
}

SCENARIO("EntityRegistry can be deserialized through the registry", "[serialization]") {
    GIVEN("an initialized serialization plugin") {
        SerializationTestContext context;
        context.init();

        auto* ser_registry = context.serialization_registry();
        auto& entry = ser_registry->get("EntityRegistry");

        cask::UUID uuid = cask::generate_uuid();
        std::string uuid_str = uuids::to_string(uuid);

        nlohmann::json data = {{uuid_str, 42}};

        WHEN("the EntityRegistry is deserialized") {
            auto result = entry.deserialize(data, context.entity_registry, nlohmann::json{});

            THEN("the result contains entity_remap mapping file IDs to runtime IDs") {
                REQUIRE(result.contains("entity_remap"));
                REQUIRE(result["entity_remap"].contains("42"));
            }

            THEN("the entity is resolvable from the registry") {
                auto parsed = uuids::uuid::from_string(uuid_str);
                REQUIRE(parsed.has_value());
                REQUIRE(context.entity_registry->has(
                    result["entity_remap"]["42"].get<uint32_t>()
                ));
            }
        }

        context.shutdown();
    }
}
