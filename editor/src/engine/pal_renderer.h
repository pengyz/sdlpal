#pragma once
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "SDL.h"
#include "common.h"
#include <array>

namespace render {
class RenderBackend;
}

namespace engine {

using PaletteColors = std::array<SDL_Color, 256>;

/**
 * @brief 渲染器
 *
 */
class PalRenderer {
public:
    PalRenderer(SDL_Renderer* render);
    ~PalRenderer();
    /**
     * @brief 初始化
     *
     * @param window
     * @param widht
     * @param height
     * @return true
     * @return false
     */
    bool init(SDL_Window* window, int widht, int height);

    /**
     * @brief deinitialize
     *
     */
    void deinit();

    /**
     * @brief 颜色填充
     *
     * @param color
     * @param rect
     */
    void fillRect(SDL_Colour color, const SDL_Rect& rect);

    /**
     * @brief Get the specified palette in pat.mkf file.
     *
     * @param iPaletteNum number of the palette.
     * @param fNight whether use the night palette or not.
     * @return SDL_Color* Pointer to the palette. NULL if failed.
     */
    SDL_Color* getPalette(int32_t iPaletteNum, bool fNight);
    /**
     * @brief Set the palette of the screen.
     *
     * @param rgPalette array of 256 colors.
     */
    void setPalette(SDL_Color rgPalette[256]);

    void setPalette(int32_t iPaletteNum, bool fNight);

    void updateScreen(const SDL_Rect* lpRect);

    SDL_Renderer* getRenderer() const { return _renderer; }

    void present();

    void erase(int r, int g, int b, int a);

    SDL_Surface* getScreen() const { return _screen; }

    void resize(int w, int h);

    void setPaused(bool paused) { _bRenderPaused = paused; }

    SDL_Texture* getTexture() { return _texture; }

private:
    SDL_Renderer* _renderer = nullptr; // gpRenderer
    PaletteColors _paletteColors;
    SDL_Palette* _palette; // gpPalette
    SDL_Surface* _screen = nullptr; // gpScreen
    SDL_Surface* _screenBak = NULL; // gpScreenBak
    SDL_Surface* _screenReal = nullptr; // gpScreenReal: The real screen surface
    SDL_Texture* _texture = nullptr; // gpTexture
    bool _bScaleScreen = true; // g_bScaleScreen
    bool _bRenderPaused = false; // g_bRenderPaused
    // Shake times and level
    unsigned short _wShakeTime = 0; // g_wShakeTime
    unsigned short _wShakeLevel = 0; // g_wShakeLevel

    render::RenderBackend* _renderBackend = nullptr; // gRenderBackend
};
} // namespace engine