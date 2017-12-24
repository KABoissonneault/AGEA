#pragma once

#include "SDL.h"

#include <memory>

namespace hz::sdl {
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
