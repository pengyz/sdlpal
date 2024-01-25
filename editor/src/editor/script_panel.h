#pragma once

#include "window.h"

namespace engine {
class PalEngine;
}

namespace editor {

class ScriptPanel : public Window {
public:
    ScriptPanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~ScriptPanel();
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
