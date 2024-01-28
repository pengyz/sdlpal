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

struct EmemyPanelModel {
    int selected_enemy_id = -1;
};

class EnemyPanel : public Window {

public:
    EnemyPanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine);
    ~EnemyPanel();
    /**
     * @brief 渲染逻辑
     *
     */
    virtual void render() override;

    virtual bool init() override;


    /**
     * @brief update enemy sprite
     * 
     * @param wEnemyObjectID 
     */
    void updateEnemySprite(WORD wEnemyObjectID);

private:
    void drawObjectPropertyTable(WORD wEventObjectID, engine::LPEVENTOBJECT pObject);
    void drawEnemyPropertyTable(int idx, engine::OBJECT_ENEMY* pEnemy);
    void drawEnemyBattlePropertyTable(int idx, engine::LPENEMY pEnemy);

private:
    EmemyPanelModel model;
    engine::PalEngine* _engine = nullptr;
    SpritePanel* _spritePanel = nullptr;
};
} // namespace editor
