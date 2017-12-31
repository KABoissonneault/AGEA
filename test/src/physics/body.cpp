#if _MSC_VER
#define _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
#endif 

#include <catch.hpp>

#include <physics/body.h>

using namespace std::chrono_literals;

TEST_CASE("Body integration", "[physics]") {
    using hz::physics::body2d;
    using hz::physics::position2d;
    using hz::physics::velocity2d;
    using hz::physics::acceleration2d;

    SECTION("From origin") {
        auto body_origin = body2d();
        body_origin.position = position2d(0.0, 0.0);
        body_origin.velocity = velocity2d(1.0, 0.0);

        {
            auto const test_body1 = hz::physics::integrate(body_origin, 1s);
            REQUIRE(test_body1.position.value.x == Approx(body_origin.position.value.x + body_origin.velocity.value.x));
        }

        {
            auto test_body2 = body_origin;
            for(int i = 0; i < 50; ++i) {
                test_body2 = hz::physics::integrate(test_body2, 1s / 50.0);
            }
            REQUIRE(test_body2.position.value.x == Approx(body_origin.position.value.x + body_origin.velocity.value.x));
        }
    }

    SECTION("From far") {
        auto body_origin = body2d();
        body_origin.position = position2d(10000000000.0, 0.0);
        body_origin.velocity = velocity2d(1.0, 0.0);

        {
            auto const test_body1 = hz::physics::integrate(body_origin, 1s);
            REQUIRE(test_body1.position.value.x == Approx(body_origin.position.value.x + body_origin.velocity.value.x));
        }
        
        {
            auto test_body2 = body_origin;
            for(int i = 0; i < 50; ++i) {
                test_body2 = hz::physics::integrate(test_body2, 1s / 50.0);
            }
            REQUIRE(test_body2.position.value.x == Approx(body_origin.position.value.x + body_origin.velocity.value.x));
        }
    }
}