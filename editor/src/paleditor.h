#pragma once
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
    bool initImGui();
    bool initGame();

private:
    SDL_Renderer* _renderer = nullptr; // 渲染器
};