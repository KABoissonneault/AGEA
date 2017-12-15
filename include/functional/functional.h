#pragma once

namespace hz::functional {
    template<typename T>
    struct identity {
        template<typename... Args>
        auto operator()(Args...) const noexcept -> T {
            return std::forward<T>(t);
        }

        T&& t;
    };

    template<typename T>
    constexpr auto make_identity(T&& t) noexcept -> identity<T> {
        return identity<T>{std::forward<T>(t)};
    }
}