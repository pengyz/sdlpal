#pragma once

#include "3rd/SDL/include/SDL_render.h"
#include "common.h"
#include "engine/pal_global.h"
#include "engine/pal_renderer.h"
#include "imgui.h"
#include "window.h"

namespace engine {
class PalEngine;
}

namespace editor {
class SpritePanel;

struct ScenePanelModel {
    int item_current_idx = 0;
    int selected_object_id = -1;
    int object_id_to_open = -1;
};

class ScenePanel : public Window {

public:
    ScenePanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~ScenePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

    void setInspectObjectId(int objectId);
    
private:
    void drawObjectPropertyTable(WORD wEventObjectID, engine::LPEVENTOBJECT pObject);

private:
    ScenePanelModel model;
    engine::PalEngine* _engine = nullptr;
    SpritePanel* _spritePanel = nullptr;
};
} // namespace editor
