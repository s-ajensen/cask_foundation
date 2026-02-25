#include <catch2/catch_test_macros.hpp>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/identity/entity_registry.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe_entity_registry.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/resource_handle.hpp>
#include <cask/resource/resource_loader_registry.hpp>
#include <cask/resource/resource_sources.hpp>
#include <cask/foundation/register_serializable_resource.hpp>

struct TestResource {
    int value;
};

struct SerializableResourceContext {
    World world;
    WorldHandle handle;
    cask::WorldView view;
    EntityTable table;

    SerializableResourceContext()
        : handle(handle_from_world(&world))
        , view(handle) {
        auto* compactor = view.register_component<EntityCompactor>("EntityCompactor");
        compactor->table_ = &table;

        uint32_t table_id = view.register_component("EntityTable");
        world.bind(table_id, &table);

        auto* entity_registry = view.register_component<EntityRegistry>("EntityRegistry");
        view.register_component<cask::SerializationRegistry>("SerializationRegistry");

        auto entity_entry = cask::describe_entity_registry("EntityRegistry", table);
        registry()->add("EntityRegistry", entity_entry);

        view.register_component<ResourceStore<TestResource>>("TestResourceStore");
        auto* handle_store = view.register_component<ComponentStore<ResourceHandle<TestResource>>>("TestResourceComponents");
        compactor->add(handle_store, remove_component<ResourceHandle<TestResource>>);
        view.register_component<cask::ResourceLoaderRegistry<TestResource>>("TestResourceLoaderRegistry");
    }

    cask::SerializationRegistry* registry() {
        return view.resolve<cask::SerializationRegistry>("SerializationRegistry");
    }

    ResourceStore<TestResource>* resource_store() {
        return view.resolve<ResourceStore<TestResource>>("TestResourceStore");
    }

    cask::ResourceLoaderRegistry<TestResource>* loader_registry() {
        return view.resolve<cask::ResourceLoaderRegistry<TestResource>>("TestResourceLoaderRegistry");
    }
};

SCENARIO("registering a serializable resource creates ResourceSources that is resolvable", "[resource_registration]") {
    GIVEN("a world with required infrastructure") {
        SerializableResourceContext context;

        WHEN("register_serializable_resource is called") {
            auto* sources = cask::register_serializable_resource<TestResource>(
                context.view, "TestResourceStore", "TestResourceComponents",
                "TestResourceSources", "TestResourceLoaderRegistry");

            THEN("the returned pointer is not null") {
                REQUIRE(sources != nullptr);
            }

            THEN("the sources are resolvable by name from the world") {
                auto* resolved = context.view.resolve<ResourceSources<TestResource>>("TestResourceSources");
                REQUIRE(resolved == sources);
            }
        }
    }
}

SCENARIO("registering a serializable resource registers sources in SerializationRegistry", "[resource_registration]") {
    GIVEN("a world with required infrastructure") {
        SerializableResourceContext context;

        WHEN("register_serializable_resource is called") {
            cask::register_serializable_resource<TestResource>(
                context.view, "TestResourceStore", "TestResourceComponents",
                "TestResourceSources", "TestResourceLoaderRegistry");

            THEN("SerializationRegistry has an entry for the sources name") {
                REQUIRE(context.registry()->has("TestResourceSources"));
            }
        }
    }
}

SCENARIO("registering a serializable resource registers component handle store in SerializationRegistry", "[resource_registration]") {
    GIVEN("a world with required infrastructure") {
        SerializableResourceContext context;

        WHEN("register_serializable_resource is called") {
            cask::register_serializable_resource<TestResource>(
                context.view, "TestResourceStore", "TestResourceComponents",
                "TestResourceSources", "TestResourceLoaderRegistry");

            THEN("SerializationRegistry has an entry for the components name") {
                REQUIRE(context.registry()->has("TestResourceComponents"));
            }
        }
    }
}

SCENARIO("resource sources entry has no serialization dependencies", "[resource_registration]") {
    GIVEN("a world with a registered serializable resource") {
        SerializableResourceContext context;
        cask::register_serializable_resource<TestResource>(
            context.view, "TestResourceStore", "TestResourceComponents",
            "TestResourceSources", "TestResourceLoaderRegistry");

        WHEN("the sources entry is inspected") {
            auto& entry = context.registry()->get("TestResourceSources");

            THEN("its dependencies are empty") {
                REQUIRE(entry.dependencies.empty());
            }
        }
    }
}

SCENARIO("resource components entry depends on EntityRegistry and sources", "[resource_registration]") {
    GIVEN("a world with a registered serializable resource") {
        SerializableResourceContext context;
        cask::register_serializable_resource<TestResource>(
            context.view, "TestResourceStore", "TestResourceComponents",
            "TestResourceSources", "TestResourceLoaderRegistry");

        WHEN("the components entry is inspected") {
            auto& entry = context.registry()->get("TestResourceComponents");

            THEN("its dependencies include EntityRegistry and the sources name") {
                REQUIRE(entry.dependencies.size() == 2);
                REQUIRE(entry.dependencies[0] == "EntityRegistry");
                REQUIRE(entry.dependencies[1] == "TestResourceSources");
            }
        }
    }
}

SCENARIO("resource sources round-trip invokes registered loaders", "[resource_registration]") {
    GIVEN("a world with a registered serializable resource and a test loader") {
        SerializableResourceContext context;

        bool loader_invoked = false;
        context.loader_registry()->add("test", [&loader_invoked](const nlohmann::json& entry_json) {
            loader_invoked = true;
            return TestResource{entry_json["val"].get<int>()};
        });

        cask::register_serializable_resource<TestResource>(
            context.view, "TestResourceStore", "TestResourceComponents",
            "TestResourceSources", "TestResourceLoaderRegistry");

        auto* sources = context.view.resolve<ResourceSources<TestResource>>("TestResourceSources");
        sources->entries["item_a"] = {{"loader", "test"}, {"val", 42}};

        WHEN("sources are serialized then deserialized into a fresh context") {
            auto& sources_entry = context.registry()->get("TestResourceSources");
            auto serialized = sources_entry.serialize(sources);

            SerializableResourceContext fresh_context;
            fresh_context.loader_registry()->add("test", [](const nlohmann::json& entry_json) {
                return TestResource{entry_json["val"].get<int>()};
            });

            cask::register_serializable_resource<TestResource>(
                fresh_context.view, "TestResourceStore", "TestResourceComponents",
                "TestResourceSources", "TestResourceLoaderRegistry");

            auto* fresh_sources = fresh_context.view.resolve<ResourceSources<TestResource>>("TestResourceSources");
            auto& fresh_entry = fresh_context.registry()->get("TestResourceSources");
            auto remap_context = fresh_entry.deserialize(serialized, fresh_sources, nlohmann::json{});

            THEN("the ResourceStore contains the loaded resource") {
                auto* fresh_store = fresh_context.resource_store();
                auto handle_value = fresh_store->key_to_handle_.at("item_a");
                REQUIRE(fresh_store->resources_[handle_value].value == 42);
            }

            THEN("the resource_remap context is produced") {
                REQUIRE(remap_context.contains("resource_remap_TestResourceSources"));
            }
        }
    }
}
