#pragma once

#include "3rd/SDL/include/SDL_render.h"
#include "engine/pal_global.h"
#include "window.h"

namespace engine {
class PalEngine;
}

namespace editor {

struct SpritePanelModel {
    WORD spriteObjectId = -1;
    SDL_Texture* texture = nullptr;
    bool sprite_review_open = false;
    ImVec2 texture_size;
    float texture_scale = 2.0f;
    bool value_changed = false;
};

class SpritePanel : public Window {
public:
    SpritePanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~SpritePanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;

    void openWindow(WORD eventObjectId, engine::LPEVENTOBJECT pObject)
    {
        updateSpriteTexture(eventObjectId, pObject);
        _pObject = pObject;
        model.sprite_review_open = true;
    }

    void closeWindow() { model.sprite_review_open = false; }

private:
    void updateSpriteTexture(WORD eventObjectId, engine::LPEVENTOBJECT pObject);

private:
    engine::PalEngine* _engine = nullptr;
    SpritePanelModel model;
    engine::LPEVENTOBJECT _pObject = nullptr;
};
} // namespace editor
