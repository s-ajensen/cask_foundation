#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/ecs/frame_advancer.hpp>
#include <cask/ecs/interpolated.hpp>
#include <cstring>

struct InterpolationTestContext : PluginTestContext {
    FrameAdvancer* advancer() {
        uint32_t advancer_id = world.register_component("FrameAdvancer");
        return world.get<FrameAdvancer>(advancer_id);
    }
};

SCENARIO("interpolation plugin reports its metadata", "[interpolation]") {
    GIVEN("the interpolation plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is interpolation") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "interpolation") == 0);
        }

        THEN("it defines the FrameAdvancer component") {
            REQUIRE(info->defines_count == 1);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "FrameAdvancer") == 0);
        }

        THEN("it requires no components") {
            REQUIRE(info->requires_count == 0);
            REQUIRE(info->requires_components == nullptr);
        }

        THEN("it provides init tick and shutdown functions") {
            REQUIRE(info->init_fn != nullptr);
            REQUIRE(info->tick_fn != nullptr);
            REQUIRE(info->shutdown_fn != nullptr);
        }

        THEN("it does not provide a frame function") {
            REQUIRE(info->frame_fn == nullptr);
        }
    }
}

SCENARIO("interpolation plugin initializes FrameAdvancer", "[interpolation]") {
    GIVEN("a world and the interpolation plugin") {
        InterpolationTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("FrameAdvancer is registered and retrievable") {
                REQUIRE(context.advancer() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("interpolation plugin tick advances all interpolated values", "[interpolation]") {
    GIVEN("an initialized interpolation plugin with a registered interpolated value") {
        InterpolationTestContext context;
        context.init();

        Interpolated<float> interpolated{0.0f, 5.0f};
        context.advancer()->add(&interpolated, advance_interpolated<float>);

        WHEN("tick is called") {
            context.tick();

            THEN("previous is copied from current") {
                REQUIRE(interpolated.previous == 5.0f);
            }
        }

        context.shutdown();
    }
}

SCENARIO("interpolation plugin shutdown prevents tick from advancing", "[interpolation]") {
    GIVEN("an initialized interpolation plugin with a registered interpolated value") {
        InterpolationTestContext context;
        context.init();

        Interpolated<float> interpolated{0.0f, 5.0f};
        context.advancer()->add(&interpolated, advance_interpolated<float>);

        WHEN("shutdown is called and then tick is called") {
            context.shutdown();
            context.tick();

            THEN("the interpolated value is not advanced") {
                REQUIRE(interpolated.previous == 0.0f);
            }
        }
    }
}
