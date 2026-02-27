#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/resource/project_root.hpp>
#include <cstring>
#include <cstdlib>

SCENARIO("project plugin registers ProjectRoot on init", "[project]") {
    GIVEN("a fresh world with the project plugin") {
        PluginTestContext context;

        WHEN("init is called") {
            context.init();
            uint32_t root_id = context.world.register_component("ProjectRoot");
            auto* root = context.world.get<ProjectRoot>(root_id);

            THEN("ProjectRoot is registered and retrievable") {
                REQUIRE(root != nullptr);
            }

            THEN("its path is not empty") {
                REQUIRE_FALSE(root->path.empty());
            }

            context.shutdown();
        }
    }
}

SCENARIO("project plugin preserves pre-registered ProjectRoot", "[project]") {
    GIVEN("a world where ProjectRoot is already registered") {
        PluginTestContext context;
        ProjectRoot root{"/custom/root"};
        uint32_t root_id = context.world.register_component("ProjectRoot");
        context.world.bind(root_id, &root);

        WHEN("init is called") {
            context.init();
            auto* resolved = context.world.get<ProjectRoot>(root_id);

            THEN("path is still /custom/root") {
                REQUIRE(resolved->path == "/custom/root");
            }
        }
    }
}

SCENARIO("project plugin uses environment variable when set", "[project]") {
    GIVEN("CASK_PROJECT_ROOT is set to /env/override") {
        setenv("CASK_PROJECT_ROOT", "/env/override", 1);
        PluginTestContext context;

        WHEN("init is called") {
            context.init();
            uint32_t root_id = context.world.register_component("ProjectRoot");
            auto* root = context.world.get<ProjectRoot>(root_id);

            THEN("ProjectRoot path is /env/override") {
                REQUIRE(root->path == "/env/override");
            }
        }

        unsetenv("CASK_PROJECT_ROOT");
    }
}

SCENARIO("environment variable overrides pre-registered ProjectRoot", "[project]") {
    GIVEN("a world where ProjectRoot is already registered and CASK_PROJECT_ROOT is set") {
        PluginTestContext context;
        ProjectRoot root{"/custom/root"};
        uint32_t root_id = context.world.register_component("ProjectRoot");
        context.world.bind(root_id, &root);
        setenv("CASK_PROJECT_ROOT", "/env/override", 1);

        WHEN("init is called") {
            context.init();
            auto* resolved = context.world.get<ProjectRoot>(root_id);

            THEN("ProjectRoot path is /env/override") {
                REQUIRE(resolved->path == "/env/override");
            }
        }

        unsetenv("CASK_PROJECT_ROOT");
    }
}

SCENARIO("project plugin reports correct metadata", "[project]") {
    GIVEN("the project plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("name is project") {
            REQUIRE(std::strcmp(info->name, "project") == 0);
        }

        THEN("it defines ProjectRoot") {
            REQUIRE(info->defines_count == 1);
            REQUIRE(std::strcmp(info->defines_components[0], "ProjectRoot") == 0);
        }

        THEN("it requires no components") {
            REQUIRE(info->requires_count == 0);
        }

        THEN("it provides an init function") {
            REQUIRE(info->init_fn != nullptr);
        }

        THEN("it does not provide a tick function") {
            REQUIRE(info->tick_fn == nullptr);
        }

        THEN("it does not provide a frame function") {
            REQUIRE(info->frame_fn == nullptr);
        }

        THEN("it does not provide a shutdown function") {
            REQUIRE(info->shutdown_fn == nullptr);
        }
    }
}
