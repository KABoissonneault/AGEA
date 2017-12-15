#pragma once

#include <chrono>

namespace hz::physics {
    using seconds = std::chrono::duration<double>;
    using milliseconds = std::chrono::duration<double, std::milli>;
}