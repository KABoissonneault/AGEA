#pragma once

#include <variant>
#include <string_view>
#include <string>

namespace hz::input {
    enum class event_label {
        exit,

        up_pressed,
        up_released,
        down_pressed,
        down_released,
        left_pressed,
        left_released,
        right_pressed,
        right_released,
    };

    class event_t {
    public:
        using variant = std::variant<event_label, std::string>;

        event_t(event_label e) noexcept : event(e) {

        }
        explicit event_t(std::string_view s) : event(std::in_place_type<std::string>, s) {

        }

        auto operator==(event_t const& rhs) const noexcept -> bool {
            return this->event == rhs.event;
        }
        auto operator<(event_t const& rhs) const noexcept -> bool {
            return this->event < rhs.event;
        }

        auto get_value() const noexcept -> variant const& {
            return event;
        }

    private:
        variant event;
    };

    struct event_state_t {
    public:
        event_state_t() = default;
        explicit event_state_t(std::initializer_list<event_t> events)
            : events(events) {

        }

        auto has(event_label e) const noexcept -> bool {
            return std::find(events.begin(), events.end(), event_t(e)) != events.end();
        }
        auto has(std::string_view s) const noexcept -> bool {
            return std::find(events.begin(), events.end(), event_t(s)) != events.end();
        }

        auto push(event_t e) -> event_state_t & {
            events.push_back(std::move(e));
            return *this;
        }

    private:
        std::vector<event_t> events;
    };
}
