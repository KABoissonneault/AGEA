#pragma once

namespace hz {
    namespace math {
        template<typename Y, typename F, typename T, typename H>
        constexpr auto rk1(Y y1, F f, T t1, H h) noexcept {
            return y1 + f(t1, y1)*h;
        }
    }
}