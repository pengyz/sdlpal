#pragma once

#include "3rd/SDL/include/SDL_render.h"
#include "common.h"
#include "engine/pal_global.h"
#include "engine/pal_renderer.h"
#include "imgui.h"
#include "palcommon.h"
#include "window.h"

namespace engine {
class PalEngine;
}

namespace editor {
class SpritePanel;

struct SpriteDetail {
    LPSPRITE lpSprite = nullptr;
    SDL_Texture* texture = nullptr;
    WORD width = 0;
    WORD height = 0;
};

struct EmemyPanelModel {
    std::map<WORD, SpriteDetail> sprites;
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

private:
    void drawEnemyPropertyTable(int idx, engine::OBJECT_ENEMY* pEnemy);
    void drawEnemyBattlePropertyTable(int idx, engine::LPENEMY pEnemy);

    void freeSprites();

    void loadSprites(engine::LPENEMYTEAM enemyTeam);

private:
    EmemyPanelModel model;
    engine::PalEngine* _engine = nullptr;
};
} // namespace editor
