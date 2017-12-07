#pragma once

namespace hz {
    namespace math {
        struct vector2d {
            double x, y;

            constexpr auto operator+(vector2d other) const noexcept -> vector2d {
                return vector2d{ x + other.x, y + other.y };
            }
            constexpr auto operator+=(vector2d other) noexcept -> vector2d & {
                x += other.x;
                y += other.y;
                return *this;
            }
            constexpr auto operator-(vector2d other) const noexcept -> vector2d {
                return vector2d{ x - other.x, y - other.y };
            }
            constexpr auto operator-=(vector2d other) noexcept -> vector2d & {
                x -= other.x;
                y -= other.y;
                return *this;
            }
            constexpr auto operator*(double k) const noexcept -> vector2d {
                return vector2d{ this->x * k, this->y * k };
            }
        };

        constexpr auto scalar_product(vector2d lhs, vector2d rhs) -> double {
            return lhs.x*rhs.x + lhs.y*rhs.y;
        }
    }
}
