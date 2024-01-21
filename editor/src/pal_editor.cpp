#include "pal_editor.h"
#include "3rd/SDL/include/SDL.h"
#include "3rd/SDL/include/SDL_video.h"
#include "audio.h"
#include "common.h"
#include "engine/pal_engine.h"
#include "engine/pal_global.h"
#include "engine/pal_resources.h"
#include "engine/pal_scene.h"
#include "engine/pal_script.h"
#include "game.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "palcfg.h"
#include "palcommon.h"
#include "script.h"
#include "util.h"
#include <imgui.h>
#include <iostream>

extern "C" {
extern VOID PAL_InitGlobalGameData(VOID);
extern VOID PAL_LoadDefaultGame(VOID);
VOID PAL_SceneDrawSprites(VOID);
VOID PAL_DialogWaitForKey(VOID);
}

namespace editor {

PALEditor::PALEditor()
    : _engine(new engine::PalEngine())
{
}

PALEditor::~PALEditor()
{
    deinit();
}

bool PALEditor::init()
{
    if (SDL_Init(PAL_SDL_INIT_FLAGS) == -1) {
        TerminateOnError("Could not initialize SDL: %s.\n", SDL_GetError());
    }
    // load config
    PAL_LoadConfig(TRUE);

    UTIL_LogAddOutputCallback([](LOGLEVEL _, const char* string, const char* __) {
        const char* ptr = NULL;
        if ((ptr = strstr(string, "[SCRIPT] "))) {
            ptr += strlen("[SCRIPT] ");
        } else if ((ptr = strstr(string, "[AUTOSCRIPT] "))) {
            ptr += strlen("[AUTOSCRIPT] ") + 6;
        }
        if (ptr) {
            WORD wScriptEntry = 0;
            SCRIPTENTRY script;
            memset(&script, 0, sizeof(script));
            LPSCRIPTENTRY pScript = &script;
            auto ret = sscanf(ptr, "%4hx: %4hx %4hx %4hx %4hx\n", &wScriptEntry, &pScript->wOperation, &pScript->rgwOperand[0],
                &pScript->rgwOperand[1], &pScript->rgwOperand[2]);
            if (ret != 5) {
                printf("failed !\n");
                return;
            }
            printScript((WORD)wScriptEntry, pScript);
        } else {
            std::cout << string;
            if (string[strlen(string) - 1] != '\n')
                std::cout << std::endl;
        }
    },
        LOGLEVEL_DEBUG);

    _mainWindow = _engine->createWindow(1600, 900, "pal editor");
    if (!_engine->init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "initalize engine failed !");
        return false;
    }

    return true;
}

void PALEditor::deinit()
{
    if (_engine) {
        delete _engine;
        _engine = nullptr;
    }
    if (_mainWindow) {
        // do not free it, it's managed by PalEngine
        _mainWindow = nullptr;
    }
    SDL_Quit();
}

int PALEditor::runLoop()
{
    DWORD dwTime = SDL_GetTicks();

    _engine->getGlobals()->getCurrentSaveSlot() = 0;
    _engine->getGlobals()->getInMainGame() = TRUE;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    _engine->getGlobals()->getNumScene() = 1;
    _engine->getPalRenderer()->setPalette(0, false);
    bool isRunning = true;
    while (isRunning) {
        _engine->getResources()->loadResources();
        _engine->getInput()->clearKeyState();

        // process sdl
        while (!SDL_TICKS_PASSED(SDL_GetTicks(), (dwTime))) {
            if (_engine->getInput()->processEvent()) {
                isRunning = false;
                break;
            }
            SDL_Delay(1);
        }
        dwTime = SDL_GetTicks() + FRAME_TIME;
        static SDL_Rect rect = { 0, 0, SCENE_WIDTH, SCENE_HEIGHT };

        rect.x = PAL_X(_engine->getGlobals()->getViewport());
        rect.y = PAL_Y(_engine->getGlobals()->getViewport());
        // do not enter scene, just load this point
        _engine->getGlobals()->getEnteringScene() = FALSE;
        // PAL_ClearDialog(TRUE);
        SDL_Surface* pScreen = _engine->getPalRenderer()->getScreen();
        // PAL_MakeScene();
        PAL_MapBlitToSurface(_engine->getResources()->getCurrentMap(), pScreen, &rect, 0);
        PAL_MapBlitToSurface(_engine->getResources()->getCurrentMap(), pScreen, &rect, 1);
        _engine->getScene()->drawSprites();
        _engine->getScene()->updateParty(_engine->getInput()->getInputState());
        _engine->getPalRenderer()->updateScreen(nullptr);

        if (_engine->getGlobals()->getEnteringScene()) {
            _engine->getGlobals()->getEnteringScene() = FALSE;
            WORD i = _engine->getGlobals()->getNumScene() - 1;
            _engine->getGlobals()->getGameData().rgScene[i].wScriptOnEnter = PAL_RunTriggerScript(_engine->getGlobals()->getGameData().rgScene[i].wScriptOnEnter, 0xFFFF);
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
        SDL_RenderSetScale(_engine->getRenderer(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(_engine->getRenderer(), (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(_engine->getPalRenderer()->getRenderer());
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        _engine->getPalRenderer()->present();
    }
    return 0;
}

}
