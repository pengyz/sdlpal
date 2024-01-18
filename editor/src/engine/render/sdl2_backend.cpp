#include "sdl2_backend.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "SDL.h"
#include "common.h"
#include "util.h"
#include <cfloat>
#include <cmath>

namespace render {

void SDL2_Backend::Init() { UTIL_LogOutput(LOGLEVEL_INFO, "SDL2_Backend::Init()"); }

void SDL2_Backend::Setup() { UTIL_LogOutput(LOGLEVEL_INFO, "SDL2_Backend::Setup()"); }

SDL_Texture* SDL2_Backend::CreateTexture(int width, int height)
{
    int texture_width, texture_height;
    double ratio = (double)width / (double)height;
    ratio *= 1.6f * (double)_textureHeight / (float)_textureWidth;
    //
    // Check whether to keep the aspect ratio
    //
    if (keepAspectRatio() && fabs(ratio - 1.6f) > FLT_EPSILON) {
        if (ratio > 1.6f) {
            texture_height = 200;
            texture_width = (int)(200 * ratio) & ~0x3;
            ratio = (float)height / 200.0f;
        } else {
            texture_width = 320;
            texture_height = (int)(320 / ratio) & ~0x3;
            ratio = (float)width / 320.0f;
        }

        unsigned short w = (unsigned short)(ratio * 320.0f) & ~0x3;
        unsigned short h = (unsigned short)(ratio * 200.0f) & ~0x3;
        _textureRect.x = (texture_width - 320) / 2;
        _textureRect.y = (texture_height - 200) / 2;
        _textureRect.w = 320;
        _textureRect.h = 200;

        // VIDEO_SetupTouchArea(width, height, w, h);
    } else {
        texture_width = 320;
        texture_height = 200;
        _textureRect.x = _textureRect.y = 0;
        _textureRect.w = 320;
        _textureRect.h = 200;

        // VIDEO_SetupTouchArea(width, height, width, height);
    }

    //
    // Create texture for screen as a render target
    //
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        texture_width, texture_height);
    return _texture;
}

void SDL2_Backend::RenderCopy()
{
    void* texture_pixels = nullptr;
    int texture_pitch = 0;

    if(SDL_LockTexture(_texture, NULL, &texture_pixels, &texture_pitch)) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "SDL_LockTexture failed: %s",  SDL_GetError());
    }
    memset(texture_pixels, 0, _textureRect.y * texture_pitch);
    uint8_t* pixels = (uint8_t*)texture_pixels + _textureRect.y * texture_pitch;
    uint8_t* src = (uint8_t*)_screenReal->pixels;
    int left_pitch = _textureRect.x << 2;
    int right_pitch = texture_pitch - ((_textureRect.x + _textureRect.w) << 2);
    for (int y = 0; y < _textureRect.h; y++, src += _screenReal->pitch) {
        memset(pixels, 0, left_pitch);
        pixels += left_pitch;
        memcpy(pixels, src, 320 << 2);
        pixels += 320 << 2;
        memset(pixels, 0, right_pitch);
        pixels += right_pitch;
    }
    memset(pixels, 0, _textureRect.y * texture_pitch);
    SDL_UnlockTexture(_texture);

    SDL_RenderClear(_renderer);
    // SDL_RenderCopy(_renderer, _texture, NULL, NULL);
    // if (gConfig.fUseTouchOverlay) {
    //     SDL_RenderCopy(renderer(), gpTouchOverlay, NULL, &gOverlayRect);
    // }
    // SDL_RenderPresent(renderer());
}

} // namespace render
