#pragma once

#include "window.h"

namespace editor {

struct ScenePanelModel {
    int item_current_idx = 0;
};

class ScenePanel : public Window {
public:
    ScenePanel(int width, int height, const std::string& title);
    ~ScenePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

private:
  ScenePanelModel model;
};
} // namespace editor
