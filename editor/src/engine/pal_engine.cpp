#include "pal_engine.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "audio.h"
#include "common.h"
#include "editor/native_window.h"
#include "game.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "pal_common.h"
#include "script.h"
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

void MapBlitToSurface(LPCPALMAP lpMap, SDL_Surface* lpSurface, const SDL_Rect* lpSrcRect, BYTE ucLayer, bool clean)
{
    int sx, sy, dx, dy, x, y, h, xPos, yPos;
    LPCBITMAPRLE lpBitmap = NULL;

    //
    // Convert the coordinate
    //
    sy = lpSrcRect->y / 16 - 1;
    dy = (lpSrcRect->y + lpSrcRect->h) / 16 + 2;
    sx = lpSrcRect->x / 32 - 1;
    dx = (lpSrcRect->x + lpSrcRect->w) / 32 + 2;

    //
    // Do the drawing.
    //
    yPos = sy * 16 - 8 - lpSrcRect->y;
    for (y = sy; y < dy; y++) {
        for (h = 0; h < 2; h++, yPos += 8) {
            xPos = sx * 32 + h * 16 - 16 - lpSrcRect->x;
            for (x = sx; x < dx; x++, xPos += 32) {
                lpBitmap = PAL_MapGetTileBitmap((BYTE)x, (BYTE)y, (BYTE)h, ucLayer, lpMap);
                if (lpBitmap == NULL) {
                    if (ucLayer) {
                        continue;
                    }
                    lpBitmap = PAL_MapGetTileBitmap(0, 0, 0, ucLayer, lpMap);
                }
                RLEBlitToSurfaceWithShadow(lpBitmap, lpSurface, PAL_XY(xPos, yPos), false, clean);
            }
        }
    }
}

int PalEngine::runLoop()
{
    DWORD dwTime = SDL_GetTicks();

    getGlobals()->getCurrentSaveSlot() = 0;
    getGlobals()->getInMainGame() = TRUE;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    getGlobals()->getNumScene() = 1;
    getPalRenderer()->setPalette(0, false);
    bool isRunning = true;
    while (isRunning) {
        getResources()->loadResources();
        getInput()->clearKeyState();

        // process sdl
        while (!SDL_TICKS_PASSED(SDL_GetTicks(), (dwTime))) {
            if (getInput()->processEvent()) {
                isRunning = false;
                break;
            }
            SDL_Delay(1);
        }
        dwTime = SDL_GetTicks() + FRAME_TIME;
        static SDL_Rect rect = { 0, 0, SCENE_WIDTH, SCENE_HEIGHT };

        rect.x = PAL_X(getGlobals()->getViewport());
        rect.y = PAL_Y(getGlobals()->getViewport());
        // do not enter scene, just load this point
        getGlobals()->getEnteringScene() = FALSE;
        // PAL_ClearDialog(TRUE);
        SDL_Surface* pScreen = getPalRenderer()->getScreen();
        // PAL_MakeScene();
        if (_drawTileMap) {
            if (_drawTileMapLines) {
                gpHighlightWidth = 1;
                gpHighlightPaletteIndex = 0;
            }
            MapBlitToSurface(getResources()->getCurrentMap(), pScreen, &rect, 0, _drawTileLayers == 1);
            MapBlitToSurface(getResources()->getCurrentMap(), pScreen, &rect, 1, _drawTileLayers == 0);
            gpHighlightWidth = 0;
            // draw pass bitmap
            // _scene->checkObstacle(PAL_POS pos, bool fCheckEventObjects, WORD wSelfObject)
        } else {
            // paint black bg
            SDL_FillRect(pScreen, nullptr, 0);
        }
        if (_drawSprite) {
            getScene()->drawSprites();
        }
        getScene()->updateParty(getInput()->getInputState());
        getPalRenderer()->updateScreen(nullptr);

        if (getGlobals()->getEnteringScene()) {
            getGlobals()->getEnteringScene() = FALSE;
            WORD i = getGlobals()->getNumScene() - 1;
            getGlobals()->getGameData().rgScene[i].wScriptOnEnter = PAL_RunTriggerScript(getGlobals()->getGameData().rgScene[i].wScriptOnEnter, 0xFFFF);
        }

        // draw Imgui
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // render editor windows
        _mainWindow->render();

        // Rendering
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        SDL_RenderSetScale(getRenderer(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(getRenderer(), (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(getPalRenderer()->getRenderer());
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        getPalRenderer()->present();
    }
    return 0;
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