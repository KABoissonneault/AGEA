#include "SDL.h"

#include <cstdlib>
#include <chrono>
#include <thread>
#include <string_view>
#include <algorithm>
#include <optional>
#include <map>
#include <atomic>

#include <expected.hpp>
#include <gsl/span>

#include "physics/body.h"
#include "input/event.h"
#include "meta/detected.h"
#include "view/sdl/sdl.h"
#include "model/entity.h"

namespace hz {
    namespace {
        template<typename T>
        using view = gsl::span<T>;

        template<typename Container>
        auto make_view(Container && c) noexcept -> view<std::remove_reference_t<decltype(*std::forward<Container>(c).data())>> {
            return gsl::make_span(std::forward<Container>(c));
        }

        using seconds = std::chrono::duration<double>;
        using milliseconds = std::chrono::duration<double, std::milli>;        

        struct body_data {
            std::atomic<physics::body2d> value;
        };

        class view_entity_t {
        public:
            sdl::unique_texture texture;
            std::weak_ptr<body_data const> body;
        };

        struct game_model {
            std::vector<model::entity> model_entities;
            std::vector<std::shared_ptr<body_data>> model_body_data;
            std::vector<view_entity_t> view_entities;
        };

        class player_input {
        public:
            void on_update(model::entity & entity, input::event_state_t const& input) {
                auto const check_input = [input] (auto const event_up, auto const event_down, auto & flag) {
                    if(input.has(event_up)) {
                        flag = true;
                    } else if(input.has(event_down)) {
                        flag = false;
                    }
                };
                check_input(input::event_label::up_pressed, input::event_label::up_released, up_pressed);
                check_input(input::event_label::down_pressed, input::event_label::down_released, down_pressed);
                check_input(input::event_label::left_pressed, input::event_label::left_released, left_pressed);
                check_input(input::event_label::right_pressed, input::event_label::right_released, right_pressed);

                auto force = physics::force2d();
                if(up_pressed) force += physics::force2d(0, input_force);
                if(down_pressed) force += physics::force2d(0, -input_force);
                if(left_pressed) force += physics::force2d(-input_force, 0);
                if(right_pressed) force += physics::force2d(input_force, 0);
                entity.body.add_force(force);
            }

            double input_force = 20.0;

        private:
            bool up_pressed = false;
            bool down_pressed = false;
            bool left_pressed = false;
            bool right_pressed = false;
        };

        class gravity_component {
        public:
            void on_update(model::entity & entity) {
                entity.body.add_force({0, -entity.body.weight.value * 10.0});
            }
        };
        
        auto load_texture(SDL_Renderer & renderer, const char* file_path) -> tl::expected<sdl::unique_texture, int> {
            auto const surface = sdl::unique_surface(SDL_LoadBMP(file_path));
            if(!surface) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
                return tl::make_unexpected(-1);
            }

            auto texture = sdl::unique_texture(SDL_CreateTextureFromSurface(&renderer, surface.get()));
            if(!texture) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
                return tl::make_unexpected(-1);
            }

            return texture;
        }

        auto generate_white_texture(SDL_Renderer& renderer, int w, int h)-> tl::expected<sdl::unique_texture, int> {
            auto texture = sdl::unique_texture(SDL_CreateTexture(&renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h));
            if(!texture) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s", SDL_GetError());
                return tl::make_unexpected(-1);
            }

            auto const pixel_data = std::vector<uint32_t>(w*h, 0xFFFFFFFF);
            if(auto const error = SDL_UpdateTexture(texture.get(), nullptr, std::data(pixel_data), w*sizeof(uint32_t)); error < 0) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't update texture: %s", SDL_GetError());
                return tl::make_unexpected(error);
            }

            return texture;
        }

        auto init_entities(SDL_Renderer & renderer) -> tl::expected<game_model, int> {
            auto model_entities = std::vector<model::entity>(1);
            model_entities[0].components.push_back(gravity_component());
            model_entities[0].components.push_back(player_input());

            auto entity_data = std::make_shared<body_data>();

            auto texture = generate_white_texture(renderer, 32, 32);
            if(!texture) {
                return tl::make_unexpected(texture.error());
            }
            auto view_entities = std::vector<view_entity_t>(1);
            view_entities[0] = {std::move(texture).value(), entity_data};

            return game_model{std::move(model_entities), {entity_data}, std::move(view_entities)};
        }

        void update_entities(view<model::entity> model_entities, view<std::shared_ptr<body_data>> body_data, input::event_state_t const& input, physics::seconds dt) {
            for(auto & entity : model_entities) {
                for(auto & component : entity.components) {
                    component.on_update(entity, input, dt);
                }
                entity.body = physics::integrate(entity.body, dt);
                entity.body.acceleration = physics::acceleration2d();
            }

            for(ptrdiff_t i = 0; i < model_entities.size(); ++i) {
                body_data[i]->value.store(model_entities[i].body);
            }
        }

        auto constexpr window_x = 320;
        auto constexpr window_y = 240;
        auto constexpr camera_world_x = 100.0;
        auto constexpr camera_world_y = 75.0;

        auto integer_floor(double d) -> int {
            return static_cast<int>(d);
        }

        void render_entities(gsl::span<view_entity_t> view_entities, SDL_Renderer & renderer) {
            SDL_SetRenderDrawColor(&renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(&renderer);           

            for(auto const& entity : view_entities) {
                auto const body_data = entity.body.lock();
                if(!body_data) { continue; }
                auto const body = body_data->value.load();

                auto const center_x = window_x / 2;
                auto const center_y = window_y / 2;
                auto const dest_target_x = center_x + integer_floor(body.position.value.x / camera_world_x * window_x);
                auto const dest_target_y = center_y + integer_floor(-body.position.value.y / camera_world_y * window_y);

                auto const render_width = integer_floor(body.dimension.x / camera_world_x * window_x);
                auto const render_height = integer_floor(body.dimension.y / camera_world_y * window_y);

                auto const dest_target = SDL_Rect{
                    dest_target_x - render_width / 2,
                    dest_target_y - render_height / 2,
                    render_width,
                    render_height,
                };
                SDL_RenderCopy(&renderer, entity.texture.get(), nullptr, &dest_target);
            }
            SDL_RenderPresent(&renderer);
        }

        auto get_player_input(SDL_Event& e) -> std::optional<input::event_t> {
            if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_UP: return input::event_label::up_pressed;
                    case SDLK_DOWN: return input::event_label::down_pressed;
                    case SDLK_LEFT: return input::event_label::left_pressed;
                    case SDLK_RIGHT: return input::event_label::right_pressed;
                }
            } else if(e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    case SDLK_UP: return input::event_label::up_released;
                    case SDLK_DOWN: return input::event_label::down_released;
                    case SDLK_LEFT: return input::event_label::left_released;
                    case SDLK_RIGHT: return input::event_label::right_released;
                }
            }

            return std::nullopt;
        }

        auto get_events() -> input::event_state_t {
            auto event_state = input::event_state_t();

            auto event = SDL_Event();
            while(SDL_PollEvent(&event)) {
                if(event.type == SDL_QUIT || event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                    event_state.push(input::event_label::exit);
                }

                if(auto const player_input = get_player_input(event)) {
                    event_state.push(*player_input);
                }
            }

            return event_state;
        }

        void do_game_loop(SDL_Renderer& renderer, game_model & model) {
            auto constexpr frame_duration = milliseconds(1.0 / 60.0);
            auto frame_buffer = milliseconds();
            while(true) {
                auto const frame_start = std::chrono::steady_clock::now();

                auto const event_state = get_events();
                if(event_state.has(input::event_label::exit)) {
                    break;
                }

                while(frame_buffer > frame_duration) {
                    update_entities(model.model_entities, model.model_body_data, event_state, frame_duration);
                    frame_buffer -= frame_duration;
                }

                render_entities(model.view_entities, renderer);

                auto const frame_complete = std::chrono::steady_clock::now();
                auto const frame_completion_duration = frame_complete - frame_start;
                if(frame_completion_duration < frame_duration) {
                    std::this_thread::sleep_for(frame_duration - frame_completion_duration);
                }
                auto const frame_end = std::chrono::steady_clock::now();
                frame_buffer += frame_end - frame_start;
            }

        }

        auto game_loop(SDL_Renderer& renderer) -> tl::expected<tl::monostate, int> {
            auto game_result = init_entities(renderer);
            return game_result.map([&renderer] (game_model& model) { do_game_loop(renderer, model); });
        }
    }
}

namespace {
    namespace sdl = hz::sdl;

    auto sdl_init() -> int {
        if(auto const error = SDL_Init(0); error < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
            return error;
        }
        std::atexit(SDL_Quit);

        return {};
    }

    auto sdl_sub_init() -> int {
        auto const error = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
        if(error < 0) {
            SDL_Log("Unable to initialize SDL_video or SDL_events: %s", SDL_GetError());
            return error;
        }
        std::atexit([] { SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS); });
        return {};
    }

    auto make_window_and_renderer() -> tl::expected<std::pair<sdl::unique_window, sdl::unique_renderer>, int> {
        SDL_Window* window;
        SDL_Renderer* renderer;
        if(auto const error = SDL_CreateWindowAndRenderer(hz::window_x, hz::window_y, SDL_WINDOW_RESIZABLE, &window, &renderer)
           ; error < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
            return tl::make_unexpected(error);
        }

        return std::make_pair(sdl::unique_window(window), sdl::unique_renderer(renderer));
    }
}


auto main(int argc, char* argv[]) -> int {
    (void)argc, (void)argv;
    if(auto const error = sdl_init(); error < 0) {
        return error;
    }

    if(auto const error = sdl_sub_init(); error < 0) {
        return error;
    }

    auto result = make_window_and_renderer();
    if(!result) {
        return result.error();
    }
    auto const[window, renderer] = std::move(result).value();
    (void)window;

    auto const game_result = hz::game_loop(*renderer.get());
    return game_result ? 0 : game_result.error();
}