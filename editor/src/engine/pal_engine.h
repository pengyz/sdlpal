#pragma once
#include "3rd/SDL/include/SDL_render.h"
#include "pal_battle.h"
#include "pal_fight.h"
#include "pal_global.h"
#include "pal_input.h"
#include "pal_renderer.h"
#include "pal_resources.h"
#include "pal_scene.h"
#include "pal_script.h"
#include "pal_video.h"
#include <string>

namespace editor {
class NativeWindow;
}

namespace engine {
/**
 * @brief game engine implementation
 *
 */

enum TileMapLayers {
    TileMapLayers_0,
    TileMapLayers_1,
    TileMapLayers_all,
};

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
    PalFight* getFight() { return _fight; }
    PalVideo* getVideo() { return _video; }
    PalBattle* getBattle() { return _battle; }

    SDL_Renderer* getRenderer() { return _renderer; }

    bool& getDrawSprite() { return _drawSprite; }
    bool& getDrawTileMap() { return _drawTileMap; }
    bool& getDrawTileMapLines() { return _drawTileMapLines; }
    int& getDrawTileLayers() { return _drawTileLayers; }

private:
    PalGlobals* _globals = nullptr;
    PalResources* _resources = nullptr;
    PalRenderer* _palRenderer = nullptr;
    PalScene* _scene = nullptr;
    PalInput* _input = nullptr;
    PalFight* _fight = nullptr;
    PalVideo* _video = nullptr;
    PalBattle* _battle = nullptr;
    editor::NativeWindow* _mainWindow = nullptr;

    SDL_Renderer* _renderer = nullptr;
    bool _drawSprite = true;
    bool _drawTileMap = true;
    bool _drawTileMapLines = false;
    int _drawTileLayers = TileMapLayers_all;
};
}