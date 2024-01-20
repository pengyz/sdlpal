#pragma once

#include "window.h"

namespace engine {
  class PalGlobals;
  class PalResources;
}

namespace editor {

class FilePanel : public Window {
public:
    FilePanel(int width, int height, const std::string& title, bool visible, engine::PalGlobals* globals, engine::PalResources* resources);
    ~FilePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

private:
    engine::PalGlobals* _globals = nullptr;
    engine::PalResources* _resources = nullptr;
};
} // namespace editor
