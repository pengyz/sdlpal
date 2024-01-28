#pragma once

#include "3rd/SDL/include/SDL_surface.h"

namespace engine {
class PalRenderer;

class PalVideo {
public:
    PalVideo(PalRenderer* renderer);
    ~PalVideo() = default;

    SDL_Surface* createCompatibleSizedSurface(SDL_Surface* pSource, const SDL_Rect* pSize);
    void updateSurfacePalette(SDL_Surface* pSurface);
private:
    PalRenderer* _renderer = nullptr;
};
}