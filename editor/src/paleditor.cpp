#include "paleditor.h"
#include "3rd/SDL/include/SDL.h"
#include "3rd/SDL/include/SDL_video.h"
#include "audio.h"
#include "common.h"
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
    : _globals(new engine::PalGlobals())
    , _resources(new engine::PalResources(_globals))
{
}

PALEditor::~PALEditor()
{
    deinit();
    if (_globals)
        delete _globals;
    _globals = nullptr;
    if (_resources)
        delete _resources;
    _resources = nullptr;
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

    _mainWindow = new editor::NativeWindow(_globals, _resources, 1024, 768, "pal editor");
    if (!_mainWindow->init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "initalize editorWindow failed !");
        return false;
    }

    if (!initGameEngine()) {
        TerminateOnError("Could not initialize Game !");
    }
    // only load scene and playerSprite
    _resources->setLoadFlags(engine::kLoadScene | engine::kLoadPlayerSprite);

    return true;
}

void PALEditor::deinit()
{
    ImGui::DestroyContext();
    ImGui_ImplSDL2_Shutdown();
}

int PALEditor::runLoop()
{
    DWORD dwTime = SDL_GetTicks();

    _globals->getCurrentSaveSlot() = 0;
    _globals->getInMainGame() = TRUE;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    _globals->getNumScene() = 15;
    _mainWindow->getPalRenderer()->setPalette(0, false);
    while (TRUE) {
        _resources->loadResources();
        _mainWindow->getInput()->clearKeyState();

        // process sdl
        while (!SDL_TICKS_PASSED(SDL_GetTicks(), (dwTime))) {
            _mainWindow->getInput()->processEvent();
            SDL_Delay(1);
        }
        dwTime = SDL_GetTicks() + FRAME_TIME;
        static SDL_Rect rect = { 0, 0, 320, 200 };
        _globals->getViewport() = PAL_XY(592, 690);
        rect.x = PAL_X(_globals->getViewport());
        rect.y = PAL_Y(_globals->getViewport());
        // do not enter scene, just load this point
        _globals->getEnteringScene() = FALSE;
        // PAL_ClearDialog(TRUE);
        SDL_Surface* pScreen = _mainWindow->getPalRenderer()->getScreen();
        // PAL_MakeScene();
        PAL_MapBlitToSurface(_resources->getCurrentMap(), pScreen, &rect, 0);
        PAL_MapBlitToSurface(_resources->getCurrentMap(), pScreen, &rect, 1);
        _mainWindow->getScene()->drawSprites();
        // PAL_ShowDialogText(PAL_GetMsg(1885));
        // PAL_ClearDialog(TRUE);
        // PAL_StartDialog(kDialogLower, (BYTE)0, 39, false);
        _mainWindow->getPalRenderer()->updateScreen(nullptr);

        if (_globals->getEnteringScene()) {
            _globals->getEnteringScene() = FALSE;
            WORD i = _globals->getNumScene() - 1;
            _globals->getGameData().rgScene[i].wScriptOnEnter = PAL_RunTriggerScript(_globals->getGameData().rgScene[i].wScriptOnEnter, 0xFFFF);
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
        SDL_RenderSetScale(_mainWindow->getRenderer(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(_mainWindow->getRenderer(), (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(_mainWindow->getPalRenderer()->getRenderer());
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        _mainWindow->getPalRenderer()->present();
    }
    return 0;
}

bool PALEditor::initGameEngine()
{
    int e;
    //
    // Initialize subsystems.
    //
    e = _globals->init();
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

    _mainWindow->getInput()->init();
    _resources->init();
    AUDIO_OpenDevice();
    // PAL_AVIInit();

    SDL_SetWindowTitle(_mainWindow->window(), "pal editor");
    // init global game data and load default game
    _globals->initGlobalGameData();
    _globals->loadDefaultGame();
    return true;
}

}
