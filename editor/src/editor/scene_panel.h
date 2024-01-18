#pragma once

#include "window.h"

namespace engine {
  class PalGlobals;
  class PalResources;
}

namespace editor {

struct ScenePanelModel {
    int item_current_idx = 0;
};

class ScenePanel : public Window {
public:
    ScenePanel(int width, int height, const std::string& title, engine::PalGlobals* globals, engine::PalResources* resources);
    ~ScenePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

private:
    ScenePanelModel model;
    engine::PalGlobals* _globals = nullptr;
    engine::PalResources* _resources = nullptr;
};
} // namespace editor
