#pragma once

#include "window.h"

namespace engine {
class PalRenderer;
class PalInput;
}

namespace editor {

class GamePanel : public Window {
public:
    GamePanel(int width, int height, const std::string& title, bool visible, engine::PalRenderer* renderer, engine::PalInput* input);
    ~GamePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

private:
    engine::PalRenderer* _renderer = nullptr;
    engine::PalInput* _input = nullptr;
    int w = 0, h = 0;
};
} // namespace editor
