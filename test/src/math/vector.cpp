#if _MSC_VER
#define _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
#endif 

#include <catch.hpp>

#include <math/vector.h>

TEST_CASE("Math vector default", "[math]") {
    auto const default_vector = hz::math::vector2d();
    
    REQUIRE(default_vector.x == 0.0);
    REQUIRE(default_vector.y == 0.0);
    REQUIRE(default_vector == default_vector);
    REQUIRE(default_vector == hz::math::vector2d());
    REQUIRE(default_vector == +default_vector);
    REQUIRE(default_vector == -default_vector);
    REQUIRE(default_vector + default_vector == default_vector);
    REQUIRE(default_vector - default_vector == default_vector);
    REQUIRE(default_vector * 1.0 == default_vector);
    REQUIRE(default_vector * 100.0 == default_vector);
    REQUIRE(default_vector * -1.0 == default_vector);
    REQUIRE(default_vector * -100.0 == default_vector);
    REQUIRE(default_vector / 1.0 == default_vector);
    REQUIRE(default_vector / 100.0 == default_vector);
    REQUIRE(default_vector / -1.0 == default_vector);
    REQUIRE(default_vector / -100.0 == default_vector);
    REQUIRE(hz::math::vector2d{default_vector} == default_vector);
}

TEST_CASE("Math vector value", "[math]") {
    using hz::math::vector2d;

    auto const value_vector = vector2d{1.0,1.0};

    REQUIRE(value_vector.x == 1.0);
    REQUIRE(value_vector.y == 1.0);
    REQUIRE(value_vector == value_vector);
    REQUIRE(value_vector == vector2d{1.0, 1.0});
    REQUIRE(value_vector == +value_vector);
    REQUIRE(value_vector != -value_vector);
    REQUIRE(value_vector + value_vector == 2*value_vector);
    REQUIRE(value_vector + vector2d{0.5, 0.5} == vector2d{value_vector.x + 0.5, value_vector.y + 0.5});
    REQUIRE(vector2d{0.5, 0.5} + value_vector == vector2d{value_vector.x + 0.5, value_vector.y + 0.5});
    REQUIRE(value_vector - value_vector == vector2d{});
    REQUIRE(value_vector - vector2d{0.5, 0.5} == vector2d{value_vector.x - 0.5, value_vector.y - 0.5});
    REQUIRE(vector2d{0.5, 0.5} - value_vector == vector2d{0.5 - value_vector.x, 0.5 - value_vector.y});
    REQUIRE(value_vector * 1.0 == value_vector);
    REQUIRE(1.0 * value_vector == value_vector);
    REQUIRE(value_vector * 100.0 == vector2d{value_vector.x * 100.0, value_vector.y * 100.0});
    REQUIRE(100.0 * value_vector == vector2d{value_vector.x * 100.0, value_vector.y * 100.0});
    REQUIRE(value_vector * -1.0 == -value_vector);
    REQUIRE(-1.0 * value_vector == -value_vector);
    REQUIRE(value_vector * -100.0 == vector2d{value_vector.x * -100.0, value_vector.y * -100.0});
    REQUIRE(-100.0 * value_vector == vector2d{value_vector.x * -100.0, value_vector.y * -100.0});
    REQUIRE(value_vector / 1.0 == value_vector);
    REQUIRE(value_vector / 100.0 == vector2d{value_vector.x / 100.0, value_vector.y / 100.0});
    REQUIRE(value_vector / -1.0 == -value_vector);
    REQUIRE(value_vector / -100.0 == vector2d{value_vector.x / -100.0, value_vector.y / -100.0});
    REQUIRE(vector2d{value_vector} == value_vector);

}