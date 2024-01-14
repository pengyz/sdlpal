#pragma once
#include "3rd/SDL/include/SDL_surface.h"
#include "render_backend.h"

namespace render {
class SDL2_Backend : public RenderBackend {
public:
    SDL2_Backend(SDL_Renderer* renderer, SDL_Surface* surface, bool keepAspectRatio)
        : RenderBackend(renderer, surface, keepAspectRatio)
    {
    }
    void Init() override;
    void Setup() override;
    SDL_Texture* CreateTexture(int width, int height) override;
    void RenderCopy() override;
};
} // namespace render