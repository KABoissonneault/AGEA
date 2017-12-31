#pragma once

namespace hz::math {
    struct vector2d {
        double x, y;

        constexpr auto operator+() const noexcept -> vector2d {
            return vector2d{x, y};
        }
        constexpr auto operator+=(vector2d other) noexcept -> vector2d & {
            x += other.x;
            y += other.y;
            return *this;
        }
        constexpr auto operator-() const noexcept -> vector2d {
            return vector2d{-x, -y};
        }
        
        constexpr auto operator-=(vector2d other) noexcept -> vector2d & {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        
        constexpr auto operator==(vector2d other) const noexcept -> bool {
            return x == other.x && y == other.y;
        }
        constexpr auto operator!=(vector2d other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    inline constexpr auto operator+(vector2d lhs, vector2d rhs) noexcept -> vector2d {
        return vector2d{lhs.x + rhs.x, lhs.y + rhs.y};
    }
    constexpr auto operator-(vector2d lhs, vector2d rhs) noexcept -> vector2d {
        return vector2d{lhs.x - rhs.x, lhs.y - rhs.y};
    }
    inline constexpr auto operator*(vector2d lhs, double rhs) noexcept -> vector2d {
        return vector2d{lhs.x * rhs, lhs.y * rhs};
    }
    inline constexpr auto operator*(double lhs, vector2d rhs) noexcept -> vector2d {
        return vector2d{lhs * rhs.x, lhs * rhs.y};
    }
    inline constexpr auto operator/(vector2d lhs, double rhs) noexcept -> vector2d {
        return vector2d{lhs.x / rhs, lhs.y / rhs};
    }

    constexpr auto scalar_product(vector2d lhs, vector2d rhs) -> double {
        return lhs.x*rhs.x + lhs.y*rhs.y;
    }
}
