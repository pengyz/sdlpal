#include "pal_video.h"
#include "pal_renderer.h"

namespace engine {

PalVideo::PalVideo(PalRenderer* renderer)
    : _renderer(renderer)
{
}

SDL_Surface* PalVideo::createCompatibleSizedSurface(SDL_Surface* pSource, const SDL_Rect* pSize)
{
    //
    // Create the surface
    //
    SDL_Surface* dest = SDL_CreateRGBSurface(pSource->flags,
        pSize ? pSize->w : pSource->w,
        pSize ? pSize->h : pSource->h,
        pSource->format->BitsPerPixel,
        pSource->format->Rmask, pSource->format->Gmask,
        pSource->format->Bmask, pSource->format->Amask);

    if (dest) {
        updateSurfacePalette(dest);
    }

    return dest;
}

void PalVideo::updateSurfacePalette(SDL_Surface* pSurface)
{
    SDL_SetSurfacePalette(pSurface, _renderer->getPalette());
}

}