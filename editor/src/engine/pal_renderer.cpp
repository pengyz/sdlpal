#include "pal_renderer.h"
#include "3rd/SDL/include/SDL_pixels.h"
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "3rd/SDL/include/SDL_video.h"
#include "palette.h"
#include "render/sdl2_backend.h"
#include "util.h"
#include "video.h"
#include <array>

namespace engine {

#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_SoftStretch SDL_UpperBlit
#endif

PalRenderer::PalRenderer(SDL_Renderer* render)
    : _renderer(render)
{
}

PalRenderer::~PalRenderer()
{
    if (_renderBackend) {
        delete _renderBackend;
        _renderBackend = nullptr;
    }
    deinit();
}

bool PalRenderer::init(int width, int height)
{
    // create surfaces
    _screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCENE_WIDTH, SCENE_HEIGHT, 8, 0, 0, 0, 0);
    _screenBak = SDL_CreateRGBSurface(SDL_SWSURFACE, SCENE_WIDTH, SCENE_HEIGHT, 8, 0, 0, 0, 0);
    _screenReal = SDL_CreateRGBSurface(SDL_SWSURFACE, SCENE_WIDTH, SCENE_HEIGHT, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    // create render backend
    _renderBackend = new render::SDL2_Backend(_renderer, _screenReal, true);
    _renderBackend->Init();
    _renderBackend->Setup();

    // int render_w, render_h;
    // SDL_GetRendererOutputSize(_renderer, &render_w, &render_h);
    //  if (!gConfig.fEnableGLSL)
    //      SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, gConfig.pszScaleQuality);
    _texture = _renderBackend->CreateTexture(width, height);
    //   if (!gConfig.fEnableGLSL)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, nullptr);

    // Create the screen buffer and the backup screen buffer.
    //
    _palette = SDL_AllocPalette(256);

    if (_screen == nullptr || _screenBak == nullptr || _screenReal == nullptr || _texture == nullptr || _palette == nullptr) {
        deinit();
        return false;
    }

    return true;
}

void PalRenderer::deinit()
{
    if (_screen != NULL) {
        SDL_FreeSurface(_screen);
    }
    _screen = NULL;

    if (_screenBak != NULL) {
        SDL_FreeSurface(_screenBak);
    }
    _screenBak = NULL;

#if SDL_VERSION_ATLEAST(2, 0, 0)
    if (_texture) {
        SDL_DestroyTexture(_texture);
    }
    _texture = NULL;
    // do not free _renderer, it's managed outside
    _renderer = NULL;

    // if (_window)
    // {
    //    SDL_DestroyWindow(_window);
    // }
    // _window = NULL;

    if (_palette) {
        SDL_FreePalette(_palette);
    }
#endif
    _palette = NULL;

    if (_screenReal != NULL) {
        SDL_FreeSurface(_screenReal);
    }
    _screenReal = NULL;
}

void PalRenderer::fillRect(SDL_Colour color, const SDL_Rect& rect)
{
    SDL_SetRenderTarget(_renderBackend->renderer(), _renderBackend->texture());
    SDL_SetRenderDrawColor(_renderBackend->renderer(), color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(_renderBackend->renderer(), &rect);
    SDL_SetRenderTarget(_renderBackend->renderer(), nullptr);
}

SDL_Color* PalRenderer::getPalette(int32_t iPaletteNum, bool fNight)
{
    return PAL_GetPalette(iPaletteNum, fNight);
}

void PalRenderer::setPalette(SDL_Color rgPalette[256])
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Rect rect;

    SDL_SetPaletteColors(_palette, rgPalette, 0, 256);

    SDL_SetSurfacePalette(_screen, _palette);
    SDL_SetSurfacePalette(_screenBak, _palette);

    //
    // HACKHACK: need to invalidate _screen->map otherwise the palette
    // would not be effective during blit
    //
    SDL_SetSurfaceColorMod(_screen, 0, 0, 0);
    SDL_SetSurfaceColorMod(_screen, 0xFF, 0xFF, 0xFF);
    SDL_SetSurfaceColorMod(_screenBak, 0, 0, 0);
    SDL_SetSurfaceColorMod(_screenBak, 0xFF, 0xFF, 0xFF);

    rect.x = 0;
    rect.y = 0;
    rect.w = SCENE_WIDTH;
    rect.h = SCENE_HEIGHT;

    updateScreen(&rect);
#else
    SDL_SetPalette(_screen, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
    SDL_SetPalette(_screenBak, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
    SDL_SetPalette(_screenReal, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
#if defined(PAL_FORCE_UPDATE_ON_PALETTE_SET)
    {
        static UINT32 time = 0;
        if (SDL_GetTicks() - time > 50) {
            SDL_UpdateRect(_screenReal, 0, 0, _screenReal->w, _screenReal->h);
            time = SDL_GetTicks();
        }
    }
#endif
#endif
}

void PalRenderer::setPalette(int32_t iPaletteNum, bool fNight)
{
    SDL_Color* p = PAL_GetPalette(iPaletteNum, fNight);

    if (p != NULL) {
        setPalette(p);
    }
}

void PalRenderer::updateScreen(const SDL_Rect* lpRect)
{
    SDL_Rect srcrect, dstrect;
    short offset = 240 - 200;
    short screenRealHeight = _screenReal->h;
    short screenRealY = 0;

#if SDL_VERSION_ATLEAST(2, 0, 0)
    if (_bRenderPaused) {
        return;
    }
#endif

    //
    // Lock surface if needed
    //
    if (SDL_MUSTLOCK(_screenReal)) {
        if (SDL_LockSurface(_screenReal) < 0)
            return;
    }

    if (!_bScaleScreen) {
        screenRealHeight -= offset;
        screenRealY = offset / 2;
    }

    if (lpRect != NULL) {
        dstrect.x = (SHORT)((INT)(lpRect->x) * _screenReal->w / _screen->w);
        dstrect.y = (SHORT)((INT)(screenRealY + lpRect->y) * screenRealHeight / _screen->h);
        dstrect.w = (WORD)((DWORD)(lpRect->w) * _screenReal->w / _screen->w);
        dstrect.h = (WORD)((DWORD)(lpRect->h) * screenRealHeight / _screen->h);

        SDL_SoftStretch(_screen, (SDL_Rect*)lpRect, _screenReal, &dstrect);
    } else if (_wShakeTime != 0) {
        //
        // Shake the screen
        //
        srcrect.x = 0;
        srcrect.y = 0;
        srcrect.w = SCENE_WIDTH;
        srcrect.h = SCENE_WIDTH - _wShakeLevel;

        dstrect.x = 0;
        dstrect.y = screenRealY;
        dstrect.w = SCENE_WIDTH * _screenReal->w / _screen->w;
        dstrect.h = (SCENE_WIDTH - _wShakeLevel) * screenRealHeight / _screen->h;

        if (_wShakeTime & 1) {
            srcrect.y = _wShakeLevel;
        } else {
            dstrect.y = (screenRealY + _wShakeLevel) * screenRealHeight / _screen->h;
        }

        SDL_SoftStretch(_screen, &srcrect, _screenReal, &dstrect);

        if (_wShakeTime & 1) {
            dstrect.y = (screenRealY + screenRealHeight - _wShakeLevel) * screenRealHeight / _screen->h;
        } else {
            dstrect.y = screenRealY;
        }

        dstrect.h = _wShakeLevel * screenRealHeight / _screen->h;

        SDL_FillRect(_screenReal, &dstrect, 0);

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
        dstrect.x = dstrect.y = 0;
        dstrect.w = _screenReal->w;
        dstrect.h = _screenReal->h;
#endif
        _wShakeTime--;
    } else {
        dstrect.x = 0;
        dstrect.y = screenRealY;
        dstrect.w = _screenReal->w;
        dstrect.h = screenRealHeight;

        SDL_SoftStretch(_screen, NULL, _screenReal, &dstrect);

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
        dstrect.x = dstrect.y = 0;
        dstrect.w = _screenReal->w;
        dstrect.h = _screenReal->h;
#endif
    }

#if SDL_VERSION_ATLEAST(2, 0, 0)
    _renderBackend->RenderCopy();
#else
    SDL_UpdateRect(_screenReal, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
#endif

    if (SDL_MUSTLOCK(_screenReal)) {
        SDL_UnlockSurface(_screenReal);
    }
}

void PalRenderer::present()
{
    SDL_RenderPresent(_renderer);
}

void PalRenderer::erase(int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(_renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
    SDL_RenderClear(_renderer);
}

VOID PalRenderer::resize(int w, int h)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Rect rect;

    if (_texture) {
        SDL_DestroyTexture(_texture);
    }

    _texture = _renderBackend->CreateTexture(w, h);

    if (_texture == NULL) {
        TerminateOnError("Re-creating texture failed on window resize!\n");
    }

    rect.x = 0;
    rect.y = 0;
    rect.w = SCENE_WIDTH;
    rect.h = SCENE_HEIGHT;

    updateScreen(&rect);
#else
    DWORD flags;
    PAL_LARGE SDL_Color palette[256];
    int i, bpp;

    //
    // Get the original palette.
    //
    if (_screenReal->format->palette != NULL) {
        for (i = 0; i < _screenReal->format->palette->ncolors; i++) {
            palette[i] = _screenReal->format->palette->colors[i];
        }
    } else
        i = 0;

    //
    // Create the screen surface.
    //
    flags = _screenReal->flags;
    bpp = _screenReal->format->BitsPerPixel;

    SDL_FreeSurface(_screenReal);
    _screenReal = SDL_SetVideoMode(w, h, bpp, flags);

    if (_screenReal == NULL) {
        //
        // Fall back to software windowed mode in default size.
        //
        _screenReal = SDL_SetVideoMode(PAL_DEFAULT_WINDOW_WIDTH, PAL_DEFAULT_WINDOW_HEIGHT, bpp, SDL_SWSURFACE);
    }

    SDL_SetPalette(_screenReal, SDL_PHYSPAL | SDL_LOGPAL, palette, 0, i);
    VIDEO_UpdateScreen(NULL);

    _palette = _screenReal->format->palette;
#endif
}

} // namespace engine
