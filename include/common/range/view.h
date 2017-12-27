#pragma once

#include <gsl/span>
#include <utility>

namespace hz::range {
    template<typename T>
    using contiguous_view = gsl::span<T>;

    template<typename Container>
    auto make_contiguous_view(Container && c) noexcept -> contiguous_view<std::remove_reference_t<decltype(std::data(*std::forward<Container>(c)))>> {
        return gsl::make_span(std::forward<Container>(c));
    }
}