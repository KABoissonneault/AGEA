#pragma once

#include <variant>
#include <string_view>
#include <string>

namespace hz {
    namespace input {
        enum class event_label {
            exit
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
    }
}
