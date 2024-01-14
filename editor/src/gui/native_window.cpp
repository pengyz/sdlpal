#include "native_window.h"
#include "3rd/SDL/include/SDL_error.h"
#include "3rd/SDL/include/SDL_render.h"
#include "SDL.h"
#include "common.h"
#include "game_panel.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "pal_config.h"
#include "palcfg.h"
#include "scene_panel.h"
#include "util.h"
#include "window.h"
#include <cassert>

extern SDL_Window* gpWindow;
extern SDL_Renderer* gpRenderer;
extern void (*g_outside_event_handler)(const SDL_Event*);

namespace editor {

NativeWindow::NativeWindow(int width, int height, const std::string& title)
    : Window(width, height, title)
{
}

bool NativeWindow::init()
{
    bool bOk = true;
    // use my window
    _window = SDL_CreateWindow(_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width,
        _height, PAL_VIDEO_INIT_FLAGS);
    if (!_window) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "SDL_CreateWindow Failed: %s", SDL_GetError());
        return false;
    }
    // gpRenderer = _renderer;
    SDL_AddEventWatch(&NativeWindow::resizingEventWatcher, this);
    //创建逻辑texture
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    bOk = _initImGui(_renderer);
    if (!bOk) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "initImGui failed !");
        return false;
    }

    //创建窗口
    createImGuiPanel<ScenePanel>(SubPanels::scene, 800, 600, "scenes");
    createImGuiPanel<GamePanel>(SubPanels::game, nullptr, 320, 200, "game");
    return true;
}

void NativeWindow::render()
{
    //主窗口菜单项
    _paintMainMenuBar();

    //创建dock space
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);

    for (auto w : _imgui_panels) {
        if (w.second->visible())
            w.second->render();
    }
}

void NativeWindow::_paintMainMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File", "Alt + o", nullptr)) {
                UTIL_LogOutput(LOGLEVEL_ERROR, "Open File clicked !");
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Open a new file");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Layout")) {
            if (ImGui::MenuItem("Reset", "Alt + r", nullptr)) {
                UTIL_LogOutput(LOGLEVEL_INFO, "Reset all panels.");
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Reset all editor panels");
            }
            if (ImGui::MenuItem("Scene Panel", "Alt + n", &_model._scene_panel)) {
                UTIL_LogOutput(LOGLEVEL_INFO, "show file panel.");
                if (_model._scene_panel != _imgui_panels[SubPanels::scene]->visible()) {
                    _imgui_panels[SubPanels::scene]->visible(_model._scene_panel);
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Reset all editor panels");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

bool NativeWindow::_initImGui(SDL_Renderer* renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup SDL_Renderer instance
    // SDL_RendererInfo info;
    // SDL_GetRendererInfo(_gameRender->getRenderer(), &info);
    // SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.IniFilename = "gui.ini";

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(SDL_RenderGetWindow(renderer), renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

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

    // setup process event
    g_outside_event_handler = [](const SDL_Event* evt) -> void {
        ImGui_ImplSDL2_ProcessEvent(evt);
    };

    return true;
}

NativeWindow::~NativeWindow()
{
    for (const auto& pair : _imgui_panels) {
        delete pair.second;
    }
    _imgui_panels.clear();
    ImGui::DestroyContext();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
}

int NativeWindow::resizingEventWatcher(void* data, SDL_Event* event)
{
    auto window = static_cast<NativeWindow*>(data);
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
        if (win == window->_window) {
            window->_width = event->window.data1;
            window->_height = event->window.data2;
        }
    }
    return 0;
}

// void NativeWindow::present() { _gameRender->present(); }

// void NativeWindow::erase(int r, int g, int b, int a)
// {
//     _gameRender->erase(r, g, b, a);
// }

} // namespace editor
