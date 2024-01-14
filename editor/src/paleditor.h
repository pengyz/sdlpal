#pragma once
#include "engine/game_renderer.h"
#include "gui/native_window.h"
#include <SDL.h>

class PALEditor {
public:
    PALEditor() = default;
    ~PALEditor();
    bool init();

    /**
     * @brief run editor loop
     *
     * @return int
     */
    int runLoop();

    /**
     * @brief deinit editor
     *
     */
    void deinit();

private:
    bool initGameEngine();

private:
    editor::NativeWindow* _editorWindow = nullptr;
    engine::GameRenderer* _gameRender = nullptr; // 游戏渲染器
};