#pragma once
#include "common.h"
#include "log_panel.h"
#include "util.h"
#include "window.h"
#include <cassert>
#include <map>
#include <string>

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

namespace engine {
class PalRenderer;
class PalGlobals;
class PalResources;
class PalInput;
class PalScene;
class PalEngine;
}

enum class SubPanels {
    file,
    scene,
    game,
    script,
    sprites,
    log,
};

namespace editor {

/**
 * @brief Model类，imgui窗口的model数据
 *
 */
class EditorModel {
public:
    bool _file_panel = true; // 默认显示
    bool _scene_panel = true; //默认显示
    bool _script_panel = false; // 默认不显示
    bool _demo_window = false; // demo 默认不显示
    bool _log_panel = true; // log 默认展示
};

class NativeWindow : public Window {
    friend class Game;

public:
    NativeWindow(engine::PalEngine* engine, int width, int height, const std::string& title);
    ~NativeWindow() override;

    bool init() override;

    template <typename T, typename... Args>
    T* createImGuiPanel(SubPanels key, Args... args)
    {
        assert(_imgui_panels.find(key) == _imgui_panels.end());
        T* w = new T(this, args...);
        if (!w->init()) {
            addLog(editor::LogLevel::error, "init window failed !");
            delete w;
            return nullptr;
        }
        _imgui_panels[key] = w;
        return w;
    }

    template <typename T>
    T* getImGuiPanel(SubPanels key)
    {
        if (!_imgui_panels.count(key))
            return nullptr;
        return dynamic_cast<T*>(_imgui_panels[key]);
    }

    void render() override;

    SDL_Window* window() { return _window; }

    engine::PalEngine* getEngine() { return _engine; }

    bool addLog(editor::LogLevel level, const char* fmt, ...) IM_FMTARGS(3);

protected:
    static int resizingEventWatcher(void* data, SDL_Event* event);
    bool _initImGui(SDL_Renderer* renderer);
    void _paintMainMenuBar();

protected:
    SDL_Window* _window = nullptr; // SDL窗口
    SDL_Renderer* _renderer = nullptr; // SDL render
    std::string _title; // 标题
    EditorModel _model; // ui model
    std::map<SubPanels, editor::Window*> _imgui_panels; // imgui panels
    engine::PalEngine* _engine = nullptr;
};
} // namespace editor
