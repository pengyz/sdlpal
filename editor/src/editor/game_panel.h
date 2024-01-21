#pragma once

#include "window.h"

namespace engine {
class PalEngine;
}

namespace editor {

class GamePanel : public Window {
public:
    GamePanel(int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~GamePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

private:
    engine::PalEngine* _engine = nullptr;
};
} // namespace editor
