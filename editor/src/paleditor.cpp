#include "paleditor.h"
#include "3rd/SDL/include/SDL_error.h"
#include "3rd/SDL/include/SDL_events.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "3rd/SDL/include/SDL_video.h"
#include "audio.h"
#include "aviplay.h"
#include "common.h"
#include "engine/pal_script.h"
#include "font.h"
#include "game.h"
#include "global.h"
#include "imgui_impl_sdlrenderer2.h"
#include "input.h"
#include "palcfg.h"
#include "palcommon.h"
#include "palette.h"
#include "play.h"
#include "res.h"
#include "scene.h"
#include "script.h"
#include "text.h"
#include "uigame.h"
#include "util.h"
#include "video.h"
#include <cstdio>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <iostream>

extern "C" {
extern VOID PAL_InitGlobalGameData(VOID);
extern VOID PAL_LoadDefaultGame(VOID);
VOID PAL_SceneDrawSprites(VOID);
VOID PAL_DialogWaitForKey(VOID);

#ifndef USE_GAME_RENDERER
extern SDL_Surface* gpScreenReal;
extern SDL_Surface* gpScreenBak;
extern SDL_Window* gpWindow;
extern SDL_Renderer* gpRenderer;
#endif
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

    _mainWindow = new editor::NativeWindow(1024, 768, "pal editor");
    if (!_mainWindow->init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "initalize editorWindow failed !");
        return false;
    }
    // initialize gameRender
    _gameRender = new engine::GameRenderer(_mainWindow->getRenderer());
    if (!_gameRender->init(_mainWindow->window(), GAME_WIDTH, GAME_HEIGHT)) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "init gameRenderer failed !");
        return false;
    }

#ifndef USE_GAME_RENDERER
    gpScreenReal = _gameRender->getScreenReal();
    gpScreen = _gameRender->getScreen();
    gpWindow = _mainWindow->window();
    gpRenderer = _gameRender->getRenderer();
    gpScreenBak = _gameRender->getScreenBak();
#endif

    if (!initGameEngine()) {
        TerminateOnError("Could not initialize Game !");
    }
    // only load scene and playerSprite
    PAL_SetLoadFlags(kLoadScene | kLoadPlayerSprite);

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

    gpGlobals->bCurrentSaveSlot = 0;
    gpGlobals->fInMainGame = TRUE;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    gpGlobals->wNumScene = 15;
#ifdef USE_GAME_RENDERER
    _gameRender->setPalette(0, true);
#else
    PAL_SetPalette(0, TRUE);
#endif
    while (TRUE) {
        PAL_LoadResources();
        PAL_ClearKeyState();

        // process sdl
        while (!SDL_TICKS_PASSED(SDL_GetTicks(), (dwTime))) {
            PAL_ProcessEvent();
            SDL_Delay(1);
        }
        dwTime = SDL_GetTicks() + FRAME_TIME;
#if 0
        PAL_StartFrame();
#else
        static SDL_Rect rect = { 0, 0, 320, 200 };
        gpGlobals->viewport = PAL_XY(592, 690);
        rect.x = PAL_X(gpGlobals->viewport);
        rect.y = PAL_Y(gpGlobals->viewport);
        // do not enter scene, just load this point
        gpGlobals->fEnteringScene = FALSE;
        PAL_ClearDialog(TRUE);
#ifdef USE_GAME_RENDERER
        SDL_Surface* pScreen = _gameRender->getScreen();
#else
        SDL_Surface* pScreen = gpScreen;
#endif
        // PAL_MakeScene();
        PAL_MapBlitToSurface(PAL_GetCurrentMap(), pScreen, &rect, 0);
        PAL_MapBlitToSurface(PAL_GetCurrentMap(), pScreen, &rect, 1);
        // PAL_SceneDrawSprites();
        // PAL_ShowDialogText(PAL_GetMsg(1885));
        // PAL_ClearDialog(TRUE);
        // PAL_StartDialog(kDialogLower, (BYTE)0, 39, false);
#ifdef USE_GAME_RENDERER
        _gameRender->updateScreen(nullptr);
#else
        VIDEO_UpdateScreen(NULL);
#endif

        if (gpGlobals->fEnteringScene) {
            gpGlobals->fEnteringScene = FALSE;
            WORD i = gpGlobals->wNumScene - 1;
            gpGlobals->g.rgScene[i].wScriptOnEnter = PAL_RunTriggerScript(gpGlobals->g.rgScene[i].wScriptOnEnter, 0xFFFF);
        }
#endif

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
        // SDL_RenderClear(_gameRender->getRenderer());
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        _gameRender->present();
    }
    return 0;
}

bool PALEditor::initGameEngine()
{
    int e;
    //
    // Initialize subsystems.
    //
    e = PAL_InitGlobals();
    if (e != 0) {
        TerminateOnError("Could not initialize global data: %d.\n", e);
    }
#ifndef USE_GAME_RENDERER
    e = VIDEO_Startup();
    if (e != 0) {
        TerminateOnError("Could not initialize Video: %d.\n", e);
    }
#endif

    VIDEO_SetWindowTitle("Loading...");

    e = PAL_InitUI();
    if (e != 0) {
        TerminateOnError("Could not initialize UI subsystem: %d.\n", e);
    }

    e = PAL_InitText();
    if (e != 0) {
        TerminateOnError("Could not initialize text subsystem: %d.\n", e);
    }

    e = PAL_InitFont(&gConfig);
    if (e != 0) {
        TerminateOnError("Could not load fonts: %d.\n", e);
    }

    PAL_InitInput();
    PAL_InitResources();
    AUDIO_OpenDevice();
    PAL_AVIInit();

    VIDEO_SetWindowTitle(UTIL_va(UTIL_GlobalBuffer(0), PAL_GLOBAL_BUFFER_SIZE,
        "PalEditor %s%s%s%s",
        gConfig.fIsWIN95 ? "Win95" : "DOS",
#if defined(_DEBUG) || defined(DEBUG)
        " (Debug) ",
#else
        "",
#endif
#if defined(PAL_HAS_GIT_REVISION) && defined(PAL_GIT_REVISION)
        " [" PAL_GIT_REVISION "] "
#else
        ""
#endif
        ,
        (gConfig.fEnableGLSL && gConfig.pszShader ? gConfig.pszShader : "")));
    // init global game data and load default game
    PAL_InitGlobalGameData();
    PAL_LoadDefaultGame();
    return true;
}