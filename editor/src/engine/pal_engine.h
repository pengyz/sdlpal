#pragma once
#include "3rd/SDL/include/SDL_render.h"
#include "pal_global.h"
#include "pal_input.h"
#include "pal_renderer.h"
#include "pal_resources.h"
#include "pal_scene.h"
#include "pal_script.h"
#include <string>

namespace editor {
class NativeWindow;
}

namespace engine {
/**
 * @brief game engine implementation
 *
 */
class PalEngine {
public:
    PalEngine();
    ~PalEngine();
    bool init();

    editor::NativeWindow* createWindow(int width, int height, const std::string& title);
    void setRenderer(SDL_Renderer* renderer);

    int runLoop();

    // getters
    PalGlobals* getGlobals() { return _globals; }
    PalResources* getResources() { return _resources; }
    PalRenderer* getPalRenderer() { return _palRenderer; }
    PalScene* getScene() { return _scene; }
    PalInput* getInput() { return _input; }
    SDL_Renderer* getRenderer() { return _renderer; }

    bool& getDrawSprite() { return _drawSprite; }
    bool& getDrawTileMap() { return _drawTileMap; }

private:
    PalGlobals* _globals = nullptr;
    PalResources* _resources = nullptr;
    PalRenderer* _palRenderer = nullptr;
    PalScene* _scene = nullptr;
    PalInput* _input = nullptr;
    editor::NativeWindow* _mainWindow = nullptr;

    SDL_Renderer* _renderer = nullptr;
    bool _drawSprite = true;
    bool _drawTileMap = true;
};
}