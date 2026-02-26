#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/ecs/frame_advancer.hpp>
#include <cask/ecs/interpolated.hpp>
#include <cstring>

struct InterpolationTestContext : PluginTestContext {
    FrameAdvancer* advancer() {
        return static_cast<FrameAdvancer*>(world.resolve("FrameAdvancer"));
    }
};

SCENARIO("interpolation plugin reports its metadata", "[interpolation]") {
    GIVEN("the interpolation plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is interpolation") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "interpolation") == 0);
        }

        THEN("it defines the FrameAdvancer and InterpolationPluginState components") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "FrameAdvancer") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "InterpolationPluginState") == 0);
        }

        THEN("it requires no components") {
            REQUIRE(info->requires_count == 0);
            REQUIRE(info->requires_components == nullptr);
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

SCENARIO("interpolation plugin isolates state between worlds", "[interpolation]") {
    GIVEN("two worlds each with the interpolation plugin initialized") {
        InterpolationTestContext world1;
        InterpolationTestContext world2;
        world1.init();
        world2.init();

        Interpolated<float> interpolated1{0.0f, 5.0f};
        Interpolated<float> interpolated2{0.0f, 10.0f};
        world1.advancer()->add(&interpolated1, advance_interpolated<float>);
        world2.advancer()->add(&interpolated2, advance_interpolated<float>);

        WHEN("only world1 is ticked") {
            world1.tick();

            THEN("world1 interpolated value is advanced") {
                REQUIRE(interpolated1.previous == 5.0f);
            }

            THEN("world2 interpolated value is not advanced") {
                REQUIRE(interpolated2.previous == 0.0f);
            }
        }

        world2.shutdown();
        world1.shutdown();
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
