#pragma once

#include "window.h"

namespace engine {
  class PalEngine;
}

namespace editor {

class FilePanel : public Window {
public:
    FilePanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~FilePanel();
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
