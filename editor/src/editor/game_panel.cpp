#include "game_panel.h"
#include "engine/pal_engine.h"
#include "imgui.h"
#include <SDL.h>
#include <iostream>

namespace editor {

GamePanel::GamePanel(int width, int height,
    const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(width, height, title)
    , _engine(engine)
{
}

GamePanel::~GamePanel() { }

void GamePanel::render()
{
    // get texture size
    int w = 0, h = 0;
    SDL_QueryTexture(_engine->getPalRenderer()->getTexture(), nullptr, nullptr, &w, &h);
    // 渲染
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    // 创建背景窗口
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::Text("游戏视图");
        _engine->getInput()->setFocus(ImGui::IsWindowFocused());
        ImGui::Image((ImTextureID)_engine->getPalRenderer()->getTexture(), { (float)w * 2.0f, (float)h * 2.0f });
    }
    ImGui::End();
}

bool GamePanel::init()
{
    return true;
}

} // namespace editor