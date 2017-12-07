#pragma once

#include "math/vector.h"
#include "math/integration.h"
#include "time.h"

#include "functional/functional.h"

namespace hz {
    namespace physics {
        using math::vector2d;

        struct displacement2d {
            displacement2d() = default;
            constexpr explicit displacement2d(double x, double y) noexcept : value{ x, y } { }
            constexpr explicit displacement2d(vector2d value) noexcept : value(value) { }

            vector2d value;
        };

        struct position2d {
            position2d() = default;
            constexpr explicit position2d(double x, double y) noexcept : value{ x,y } { }
            constexpr explicit position2d(vector2d value) noexcept : value(value) { }

            constexpr auto operator+(displacement2d other) const noexcept -> position2d {
                return position2d(value + other.value);
            }
            constexpr auto operator+=(displacement2d other) noexcept -> position2d & {
                value += other.value;
                return *this;
            }
            constexpr auto operator-(position2d other) const noexcept -> displacement2d {
                return displacement2d(value - other.value);
            }


            vector2d value;
        };

        struct velocity2d {
            velocity2d() = default;
            constexpr explicit velocity2d(double x, double y) noexcept : value{ x,y } { }
            constexpr explicit velocity2d(vector2d value) noexcept : value(value) { }

            constexpr auto operator+(velocity2d other) const noexcept -> velocity2d {
                return velocity2d(value + other.value);
            }
            constexpr auto operator+=(velocity2d other) noexcept -> velocity2d & {
                value += other.value;
                return *this;
            }
            constexpr auto operator*(seconds t) const noexcept -> displacement2d {
                return displacement2d(value * t.count());
            }

            vector2d value;
        };

        struct acceleration2d {
            acceleration2d() = default;
            constexpr explicit acceleration2d(double x, double y) noexcept : value{ x,y } { }
            constexpr explicit acceleration2d(vector2d value) noexcept : value(value) { }

            constexpr auto operator*(seconds t) const noexcept -> velocity2d {
                return velocity2d(value * t.count());
            }

            vector2d value;
        };

        struct force_t {
            force_t() = default;
            constexpr explicit force_t(double x, double y) noexcept : value{ x,y } { }
            constexpr explicit force_t(vector2d value) noexcept : value(value) { }

            vector2d value;
        };

        struct body {
            position2d position = {};
            velocity2d velocity = {};
            acceleration2d acceleration = {};
            vector2d dimension = { 1.0, 1.0 };
            double weight = 1.0;
        };

        template<typename T, typename U> 
        constexpr auto rk1(T t, U u, seconds dt) {
            return math::rk1(t, functional::make_identity(u), 0, dt);
        }

        constexpr auto integrate(body b, seconds dt) -> body {
            b.velocity = rk1(b.velocity, b.acceleration, dt / 2);
            b.position = rk1(b.position, b.velocity, dt);
            b.velocity = rk1(b.velocity, b.acceleration, dt / 2);
            return b;
        }
    }
}
