#include <SDL/SDL.h>

#include <cstdlib>
#include <chrono>
#include <thread>
#include <variant>
#include <string_view>
#include <algorithm>
#include <memory>

#include <expected.hpp>
#include <gsl/span>

namespace sdl {
	struct texture_delete {
		void operator()(SDL_Texture* p) {
			if (p != nullptr) {
				SDL_DestroyTexture(p);
			}
		}
	};

	struct surface_delete {
		void operator()(SDL_Surface* p) {
			SDL_FreeSurface(p);
		}
	};

	struct window_delete {
		void operator()(SDL_Window* p) {
			if (p != nullptr) {
				SDL_DestroyWindow(p);
			}
		}
	};

	struct renderer_delete {
		void operator()(SDL_Renderer* p) {
			if (p != nullptr) {
				SDL_DestroyRenderer(p);
			}
		}
	};

	using unique_texture = std::unique_ptr<SDL_Texture, texture_delete>;
	using unique_surface = std::unique_ptr<SDL_Surface, surface_delete>;
	using unique_window = std::unique_ptr<SDL_Window, window_delete>;
	using unique_renderer = std::unique_ptr<SDL_Renderer, renderer_delete>;
}

namespace {
	using seconds = std::chrono::duration<double>;
	using milliseconds = std::chrono::duration<double, std::milli>;

	struct vector2d {
		double x, y;

		constexpr auto operator+=(vector2d other) noexcept -> vector2d & {
			x += other.x;
			y += other.y;
			return *this;
		}
		constexpr auto operator*(double k) noexcept ->vector2d {
			return vector2d{ this->x * k, this->y * k };
		}
	};
	class pos_t {
	public:
		pos_t() = default;
		constexpr explicit pos_t(double x, double y) noexcept : value{ x,y } { }
		constexpr explicit pos_t(vector2d value) noexcept : value(value) { }
		
		constexpr auto get_x() const noexcept -> double {
			return value.x;
		}

		constexpr auto get_y() const noexcept -> double {
			return value.y;
		}

		constexpr auto operator+=(pos_t other) noexcept -> pos_t & {
			value += other.value;
			return *this;
		}
	private:
		vector2d value;
	};

	class velocity_t {
	public:
		velocity_t() = default;
		constexpr explicit velocity_t(double x, double y) noexcept : value{ x,y } { }
		constexpr explicit velocity_t(vector2d value) noexcept : value(value) { }

		constexpr pos_t operator*(seconds t) noexcept {
			return pos_t( value * t.count() );
		}

	private:
		vector2d value;
	};

	class dim_t {
	public:
		dim_t() = default;
		constexpr explicit dim_t(double x, double y) noexcept : value{ x,y } { }
		constexpr explicit dim_t(vector2d value) noexcept : value(value) { }

	private:
		vector2d value;
	};

	struct model_entity_t {
		pos_t pos;
		velocity_t velocity;
		dim_t dim;
	};

	struct view_entity_t {
		sdl::unique_texture texture;
		model_entity_t const* entity;
	};

	struct game_model {
		std::vector<model_entity_t> model_entities;
		std::vector<view_entity_t> view_entities;
	};

	auto load_texture(SDL_Renderer & renderer, const char* file_path) -> tl::expected<sdl::unique_texture, int> {
		auto const surface = sdl::unique_surface(SDL_LoadBMP(file_path));
		if (!surface) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
			return tl::make_unexpected(-1);
		}

		auto texture = sdl::unique_texture(SDL_CreateTextureFromSurface(&renderer, surface.get()));
		if (!texture) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
			return tl::make_unexpected(-1);
		}

		return texture;
	}

	auto init_entities(SDL_Renderer & renderer) -> tl::expected<game_model, int> {
		auto model_entities = std::vector<model_entity_t>(1);
		model_entities[0] = { pos_t{0.0,0.0}, velocity_t{1.0,0.0}, dim_t{1.0,1.0} };
		
		auto texture = load_texture(renderer, "assets/white.bmp");
		if (!texture) {
			return tl::make_unexpected(texture.error());
		}
		auto view_entities = std::vector<view_entity_t>(1);
		view_entities[0] = { std::move(texture).value(), &model_entities[0] };

		return game_model{ std::move(model_entities), std::move(view_entities) };
	}

	void update_entities(gsl::span<model_entity_t> model_entities, seconds dt) {
		for (auto & entity : model_entities) {
			entity.pos += entity.velocity * dt;
		}
	}

	void render_entities(gsl::span<view_entity_t> view_entities, SDL_Renderer & renderer) {
		SDL_SetRenderDrawColor(&renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(&renderer);

		auto const center_x = 320 / 2;
		auto const center_y = 240 / 2;

		for (auto const& entity : view_entities) {
			auto const dest_target_x = center_x + static_cast<int>((entity.entity->pos.get_x()) / 10.0 * 320);
			auto const dest_target_y = center_y + static_cast<int>((entity.entity->pos.get_y()) / 7.5 * 240);
			auto const dest_target = SDL_Rect{ dest_target_x - 32/2, dest_target_y - 32/2, 32, 32 };
	        SDL_RenderCopy(&renderer, entity.texture.get(), nullptr, &dest_target);
		}
		SDL_RenderPresent(&renderer);
	}

	enum class event_type {
		exit
	};

	class event_t {
		using event_variant_t = std::variant<event_type, std::string>;
	public:
		event_t() = default;
		event_t(event_type e) noexcept : event(e) {

		}
		explicit event_t(std::string_view s) : event(std::in_place_type<std::string>, s) {

		}

		template<typename InputT>
		auto operator=(InputT && input) noexcept(std::is_nothrow_assignable_v<event_variant_t, InputT>) -> event_t & {
			event = std::forward<InputT>(input);
			return *this;
		}

		auto operator==(event_t const& rhs) const noexcept
			-> bool {
			return this->event == rhs.event;
		}
		auto operator<(event_t const& rhs) const noexcept
			-> bool {
			return this->event < rhs.event;
		}
		
	private:
		event_variant_t event = {};
	};

	struct event_state_t {
	public:
		event_state_t() = default;
		explicit event_state_t(std::initializer_list<event_t> events)
			: events(events) {

		}
		
		auto has(event_type e) const noexcept -> bool {
			return std::find(events.begin(), events.end(), event_t(e)) != events.end();
		}
		auto has(std::string_view s) const noexcept -> bool {
			return std::find(events.begin(), events.end(), event_t(s)) != events.end();
		}

	private:
		std::vector<event_t> events;
	};

	event_state_t get_events() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return event_state_t{ event_type::exit };
			}
		}

		return {};
	}

	auto game_loop(SDL_Renderer& renderer) -> tl::expected<tl::monostate, int> {
		auto result = init_entities(renderer);
		if (!result) {
			return tl::make_unexpected(result.error());
		}
		auto game_model = std::move(result).value();

		auto constexpr frame_duration = milliseconds(1.0 / 60.0);
		auto frame_buffer = milliseconds();
		while (true) {
			auto const frame_start = std::chrono::steady_clock::now();

			auto const event_state = get_events();
			if (event_state.has(event_type::exit)) {
				break;
			}

			while (frame_buffer > frame_duration) {
				update_entities(game_model.model_entities, frame_duration);
				frame_buffer -= frame_duration;
			}

			render_entities(game_model.view_entities, renderer);

			auto const frame_complete = std::chrono::steady_clock::now();
			auto const frame_completion_duration = frame_complete - frame_start;
			if (frame_completion_duration < frame_duration) {
				std::this_thread::sleep_for(frame_duration - frame_completion_duration);
			}
			auto const frame_end = std::chrono::steady_clock::now();
			frame_buffer += frame_end - frame_start;
		}

		return {};
	}

	auto sdl_init() -> int {
		auto const error = SDL_Init(0);
		if (error < 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
			return error;
		}
		std::atexit(SDL_Quit);
		return {};
	}

	auto list_drivers() -> void {
		auto const driver_count = SDL_GetNumVideoDrivers();
		for (auto i = 0; i < driver_count; ++i) {
			SDL_Log("Driver: %s", SDL_GetVideoDriver(i));
		}
	}

	auto sdl_sub_init() -> int {
		auto const error = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		if (error < 0) {
			SDL_Log("Unable to initialize SDL_video or SDL_events: %s", SDL_GetError());
			return error;
		}
		std::atexit([] { SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS); });
		return {};
	}

	auto make_window_and_renderer() -> tl::expected<std::pair<sdl::unique_window, sdl::unique_renderer>, int> {
		SDL_Window* window;
		SDL_Renderer* renderer;
		if (auto const error = SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_RESIZABLE, &window, &renderer)
			; error < 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
			return tl::make_unexpected(error);
		}

		return std::make_pair(sdl::unique_window(window), sdl::unique_renderer(renderer));
	}
}

auto main(int argc, char* argv[]) -> int {
	if (auto const error = sdl_init(); error < 0) {
		return error;
	}
	list_drivers();
	if (auto const error = sdl_sub_init(); error < 0) {
		return error;
	}
	
	auto result = make_window_and_renderer();
	if (!result) {
		return result.error();
	}
	auto[window, renderer] = std::move(result).value();
	(void)window;	

	auto const game_result = game_loop(*renderer.get());
	return game_result ? 0 : game_result.error();
}