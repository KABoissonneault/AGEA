#pragma once

#include <any>
#include <vector>
#include <typeinfo>

#include "input/event.h"
#include "physics/time.h"
#include "physics/body.h"

namespace hz::model {
    class entity;

    class entity_component {
    public:
        template<typename InputT, typename = std::enable_if_t<!std::is_same<std::decay_t<InputT>, entity_component>::value>>
        entity_component(InputT&& input)
            : component_data(std::forward<InputT>(input))
            , name(typeid(InputT).name()) {

        }

        void on_update(entity & entity, input::event_state_t const& input, physics::seconds dt) {
            component_data->on_update(entity, input, dt);
        }

        auto get_name() const noexcept -> std::string_view {
            return name;
        }

    private:
        class component_interface {
        public:
            virtual auto clone() -> std::unique_ptr<component_interface> = 0;
            virtual void on_update(entity & entity, input::event_state_t const& input, physics::seconds dt) = 0;
        };

        template<typename T>
        class component_impl : public component_interface {
        public:
            template<typename... U>
            component_impl(U&&... input)
                : data(std::forward<U>(input)...) {

            }

            virtual auto clone() -> std::unique_ptr<component_interface> {
                return std::make_unique<component_impl>(component_impl{data});
            }

            template<typename U>
            using update_method_event_seconds_t = decltype(std::declval<U>().on_update(std::declval<entity&>(), std::declval<input::event_state_t>(), std::declval<seconds>()));
            template<typename U>
            using update_method_event_t = decltype(std::declval<U>().on_update(std::declval<entity&>(), std::declval<input::event_state_t>()));
            template<typename U>
            using update_method_seconds_t = decltype(std::declval<U>().on_update(std::declval<entity&>(), std::declval<physics::seconds>()));
            template<typename U>
            using update_method_empty_t = decltype(std::declval<U>().on_update(std::declval<entity&>()));

            virtual void on_update(entity & e, input::event_state_t const& input, physics::seconds dt) override {
                if constexpr(meta::is_detected<update_method_event_seconds_t, T>::value) {
                    data.on_update(e, input, dt);
                } else if constexpr(meta::is_detected<update_method_event_t, T>::value) {
                    data.on_update(e, input);
                } else if constexpr(meta::is_detected<update_method_seconds_t, T>::value) {
                    data.on_update(e, dt);
                } else if constexpr(meta::is_detected<update_method_empty_t, T>::value) {
                    data.on_update(e);
                }
            }

        private:
            T data;
        };

        class component_holder {
        public:
            component_holder() = default;
            component_holder(component_holder const& other)
                : component_data(other.component_data->clone()) {

            }
            component_holder(component_holder && other)
                : component_data(std::move(other.component_data)) {

            }
            auto operator=(component_holder const& other) -> component_holder & {
                component_data = other.component_data->clone();
                return *this;
            }
            auto operator=(component_holder && other) -> component_holder & {
                component_data = std::move(other.component_data);
                return *this;
            }
            ~component_holder() = default;

            template<typename InputT>
            component_holder(InputT&& input) 
                : component_data(make_component_data(std::forward<InputT>(input))){

            }
            
            auto operator->() -> component_interface* {
                return component_data.get();
            }
        private:
            template<typename InputT>
            static auto make_component_data(InputT&& input) -> std::unique_ptr<component_interface> {
                using impl = component_impl<std::decay_t<InputT>>;
                return std::make_unique<impl>(std::forward<InputT>(input));
            }

            std::unique_ptr<component_interface> component_data;
        };

        component_holder component_data;
        std::string_view name;
    };

    enum class id : int { };

    class entity {
    public:
        physics::body2d body;
        std::vector<entity_component> components;
        id id;
    };
}