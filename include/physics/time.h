#pragma once

#include <chrono>

namespace hz {
    namespace physics {
        using seconds = std::chrono::duration<double>;
        using milliseconds = std::chrono::duration<double, std::milli>;
    }
}