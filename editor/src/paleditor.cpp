#include "paleditor.h"
#include "3rd/SDL/include/SDL_events.h"
#include "audio.h"
#include "aviplay.h"
#include "common.h"
#include "font.h"
#include "game.h"
#include "global.h"
#include "imgui_impl_sdlrenderer2.h"
#include "input.h"
#include "palcfg.h"
#include "play.h"
#include "res.h"
#include "text.h"
#include "uigame.h"
#include "util.h"
#include "video.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>

extern SDL_Renderer* gpRenderer;
extern void (*g_outside_event_handler)(const SDL_Event*);

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

    if (!initGame()) {
        TerminateOnError("Could not initialize Game !");
    }

    // setup _render using gpRender
    _renderer = gpRenderer;

    // initialize imgui
    if (!initImGui()) {
        TerminateOnError("Could not initialize imgui !");
        return false;
    }

    // setup process event
    g_outside_event_handler = [](const SDL_Event* evt) -> void {
        ImGui_ImplSDL2_ProcessEvent(evt);
    };

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

    gpGlobals->bCurrentSaveSlot = (BYTE)PAL_OpeningMenu();
    gpGlobals->fInMainGame = TRUE;

    PAL_ReloadInNextTick(gpGlobals->bCurrentSaveSlot);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_demo_window = true;

    while (TRUE) {
        PAL_LoadResources();
        PAL_ClearKeyState();

        // process sdl
        // while (!SDL_TICKS_PASSED(SDL_GetTicks(), (dwTime))) {
            PAL_ProcessEvent();
            SDL_Delay(1);
        // }
        // dwTime = SDL_GetTicks() + FRAME_TIME;

        PAL_StartFrame();

        // draw Imgui
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);


        // Rendering
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        SDL_RenderSetScale(_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(_renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(_renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(_renderer);
    }
    return 0;
}

bool PALEditor::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup SDL_Renderer instance
    SDL_RendererInfo info;
    SDL_GetRendererInfo(_renderer, &info);
    SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.IniFilename = "gui.ini";
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(SDL_RenderGetWindow(_renderer), _renderer);
    ImGui_ImplSDLRenderer2_Init(_renderer);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple
    // fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the
    // font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in
    // your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when
    // calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to
    // write a double backslash \\ !
    io.Fonts->AddFontDefault();
    auto font = io.Fonts->AddFontFromFileTTF("./resources/wqy-micro-hei-mono.ttf", 14, nullptr,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    IM_ASSERT(font != nullptr);
    io.FontDefault = font;
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
    // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    return true;
}

bool PALEditor::initGame()
{
    int e;
#if PAL_HAS_GIT_REVISION
    UTIL_LogOutput(LOGLEVEL_DEBUG, "SDLPal build revision: %s\n", PAL_GIT_REVISION);
#endif

    //
    // Initialize subsystems.
    //
    e = PAL_InitGlobals();
    if (e != 0) {
        TerminateOnError("Could not initialize global data: %d.\n", e);
    }

    e = VIDEO_Startup();
    if (e != 0) {
        TerminateOnError("Could not initialize Video: %d.\n", e);
    }

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
    return true;
}