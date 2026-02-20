#include <catch2/catch_test_macros.hpp>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>
#include <cask/world.hpp>
#include <cask/ecs/frame_advancer.hpp>
#include <cask/ecs/interpolated.hpp>
#include <cask/foundation/register_interpolated.hpp>

SCENARIO("registering an interpolated value makes it resolvable from the world", "[registration]") {
    GIVEN("a world with a FrameAdvancer") {
        World world;
        WorldHandle handle = handle_from_world(&world);
        cask::WorldView view(handle);

        auto* advancer = view.register_component<FrameAdvancer>("FrameAdvancer");

        WHEN("register_interpolated is called") {
            auto* interpolated = cask::register_interpolated<float>(view, "FloatInterpolated");

            THEN("the interpolated value is not null") {
                REQUIRE(interpolated != nullptr);
            }

            THEN("the interpolated value is resolvable by name") {
                auto* resolved = view.resolve<Interpolated<float>>("FloatInterpolated");
                REQUIRE(resolved == interpolated);
            }
        }
    }
}

SCENARIO("registered interpolated value is wired to the FrameAdvancer", "[registration]") {
    GIVEN("a world with a FrameAdvancer and a registered interpolated value") {
        World world;
        WorldHandle handle = handle_from_world(&world);
        cask::WorldView view(handle);

        auto* advancer = view.register_component<FrameAdvancer>("FrameAdvancer");
        auto* interpolated = cask::register_interpolated<float>(view, "FloatInterpolated");

        WHEN("the interpolated value is updated and advance_all is called") {
            interpolated->current = 7.5f;
            advancer->advance_all();

            THEN("previous is updated to match current") {
                REQUIRE(interpolated->previous == 7.5f);
            }
        }
    }
}
