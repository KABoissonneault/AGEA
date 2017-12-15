#pragma once

#include <any>
#include <vector>
#include <typeinfo>

#include "input/event.h"
#include "physics/time.h"
#include "physics/body.h"

namespace hz::model {
    class entity;

    class component {
    public:
        template<typename InputT>
        component(InputT input)
            : update_function(get_update(input))
            , component_data(std::move(input))
            , name(typeid(InputT).name()) {

        }

        auto has_update() const noexcept -> bool {
            return update_function != nullptr;
        }
        void on_update(entity & entity, input::event_state_t const& input, physics::seconds dt) {
            (*update_function)(component_data, entity, input, dt);
        }

        auto get_name() const noexcept -> std::string_view {
            return name;
        }

    private:
        template<typename T>
        using update_method_event_seconds_t = decltype(std::declval<T>().on_update(std::declval<entity&>(), std::declval<input::event_state_t>(), std::declval<seconds>()));

        template<typename T>
        using update_method_event_t = decltype(std::declval<T>().on_update(std::declval<entity&>(), std::declval<input::event_state_t>()));

        template<typename T>
        using update_method_seconds_t = decltype(std::declval<T>().on_update(std::declval<entity&>(), std::declval<physics::seconds>()));

        template<typename T>
        using update_method_empty_t = decltype(std::declval<T>().on_update(std::declval<entity&>()));

        using on_update_t = void(std::any & data, entity & entity, input::event_state_t const& input, physics::seconds dt);

        template<typename InputT>
        auto get_update(InputT const&) -> on_update_t* {
            if constexpr(meta::is_detected<update_method_event_seconds_t, InputT>::value) {
                return [] (std::any & data, entity & entity, input::event_state_t const& input, seconds dt) {
                    std::any_cast<InputT&>(data).on_update(entity, input, dt);
                };
            } else if constexpr(meta::is_detected<update_method_event_t, InputT>::value) {
                return [] (std::any & data, entity & entity, input::event_state_t const& input, seconds) {
                    std::any_cast<InputT&>(data).on_update(entity, input);
                };
            } else if constexpr(meta::is_detected<update_method_seconds_t, InputT>::value) {
                return [] (std::any & data, entity & entity, input::event_state_t const&, seconds dt) {
                    std::any_cast<InputT&>(data).on_update(entity, dt);
                };
            } else if constexpr(meta::is_detected<update_method_empty_t, InputT>::value) {
                return [] (std::any & data, entity & entity, input::event_state_t const&, seconds) {
                    std::any_cast<InputT&>(data).on_update(entity);
                };
            } else {
                return nullptr;
            }
        }

        on_update_t* update_function;
        std::any component_data;
        std::string_view name;
    };

    enum class id : int { };

    class entity {
    public:
        physics::body2d body;
        std::vector<component> components;
        id id;
    };
}