#pragma once
#include "common.h"
#include "engine/game_renderer.h"
#include "util.h"
#include "window.h"
#include <cassert>
#include <map>
#include <string>

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

namespace engine {
class GameRenderer;
}

enum class SubPanels {
    scene,
    game,
    sprites
};

namespace editor {

/**
 * @brief Model类，imgui窗口的model数据
 *
 */
class EditorModel {
public:
    bool _scene_panel = true; //默认显示
};

class NativeWindow : public Window {
    friend class Game;

public:
    NativeWindow(int width, int height, const std::string& title);
    ~NativeWindow() override;

    bool init() override;

    template <typename T, typename... Args>
    T* createImGuiPanel(SubPanels key, Args... args)
    {
        assert(_imgui_panels.find(key) == _imgui_panels.end());
        T* w = new T(args...);
        if (!w->init()) {
            UTIL_LogOutput(LOGLEVEL_ERROR, "init window failed !");
            delete w;
            return nullptr;
        }
        _imgui_panels[key] = w;
        return w;
    }

    void render() override;

    SDL_Window* window() { return _window; }

    SDL_Renderer* getRenderer() const { return _renderer; }

    /**
     * @brief 颜色清屏
     *
     * @param r
     * @param g
     * @param b
     */
    // void erase(int r, int g, int b, int a);
    // /**
    //  * @brief 绘制
    //  *
    //  */
    // void present();

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
};
} // namespace editor
