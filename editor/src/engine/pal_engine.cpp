#include "pal_engine.h"
#include "audio.h"
#include "common.h"
#include "editor/native_window.h"
#include "util.h"

namespace engine {

PalEngine::PalEngine()
    : _globals(new PalGlobals())
    , _resources(new engine::PalResources(_globals))
{
}

PalEngine::~PalEngine()
{
    if (_scene)
        delete _scene;
    if (_globals)
        delete _globals;
    if (_input)
        delete _input;
    if (_palRenderer)
        delete _palRenderer;
    if (_resources)
        delete _resources;
    if (_mainWindow) {
        delete _mainWindow;
    }
}

editor::NativeWindow* PalEngine::createWindow(int width, int height, const std::string& title)
{
    _mainWindow = new editor::NativeWindow(this, width, height, title);
    if (!_mainWindow->init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "window init failed !");
        delete _mainWindow;
        return nullptr;
    }
    return _mainWindow;
}

void PalEngine::setRenderer(SDL_Renderer* renderer)
{
    _renderer = renderer;
}

bool PalEngine::init()
{
    // initialize gameRender
    _palRenderer = new engine::PalRenderer(_renderer);
    if (!_palRenderer->init(SCENE_WIDTH, SCENE_HEIGHT)) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "init gameRenderer failed !");
        return false;
    }
    _scene = new engine::PalScene(_globals, _resources, _palRenderer);
    _input = new engine::PalInput(_palRenderer);
    // only load scene and playerSprite
    _resources->setLoadFlags(engine::kLoadScene | engine::kLoadPlayerSprite);
    // init engine
    int e;
    //
    // Initialize subsystems.
    //
    e = getGlobals()->init();
    if (e != 0) {
        TerminateOnError("Could not initialize global data: %d.\n", e);
    }

#if 0
    // TODO: rewrite ui logics
    e = PAL_InitUI();
    if (e != 0) {
        TerminateOnError("Could not initialize UI subsystem: %d.\n", e);
    }
#endif

    // e = PAL_InitText();
    // if (e != 0) {
    //     TerminateOnError("Could not initialize text subsystem: %d.\n", e);
    // }

    // e = PAL_InitFont(&gConfig);
    // if (e != 0) {
    //     TerminateOnError("Could not load fonts: %d.\n", e);
    // }

    getInput()->init();
    getResources()->init();
    AUDIO_OpenDevice();
    // PAL_AVIInit();
    // init global game data and load default game
    getGlobals()->initGlobalGameData();
    getGlobals()->loadDefaultGame();
    return true;
}

}