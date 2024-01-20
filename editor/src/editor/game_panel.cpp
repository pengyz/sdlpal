#include "game_panel.h"
#include "engine/pal_input.h"
#include "engine/pal_renderer.h"
#include "imgui.h"
#include <SDL.h>
#include <iostream>

namespace editor {

GamePanel::GamePanel(int width, int height,
    const std::string& title, bool visible, engine::PalRenderer* renderer, engine::PalInput* input)
    : Window(width, height, title)
    , _renderer(renderer)
    , _input(input)
{
}

GamePanel::~GamePanel() { }

void GamePanel::render()
{
    // 渲染
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    // 创建背景窗口
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::Text("游戏视图");
        _input->setFocus(ImGui::IsWindowFocused());
        ImGui::Image((ImTextureID)_renderer->getTexture(), { (float)w * 2.0f, (float)h * 2.0f });
    }
    ImGui::End();
}

bool GamePanel::init()
{
    // get texture size
    SDL_QueryTexture(_renderer->getTexture(), nullptr, nullptr, &w, &h);
    return true;
}

} // namespace editor