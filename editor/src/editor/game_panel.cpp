#include "game_panel.h"
#include "common.h"
#include "engine/pal_engine.h"
#include "imgui.h"
#include "native_window.h"
#include "scene_panel.h"
#include <SDL.h>
#include <iostream>

bool gpHighlightHoverSprites = true;
int gpHoverPosX = 0;
int gpHoverPosY = 0;
extern int gpHoveredObject;

namespace editor {

GamePanel::GamePanel(Window* parent, int width, int height,
    const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(parent, width, height, title)
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
        ImVec2 image_pos = ImGui::GetCursorScreenPos();
        float image_width = (float)w * 2.0f;
        float image_height = (float)h * 2.0f;
        ImGui::Image((ImTextureID)_engine->getPalRenderer()->getTexture(), { image_width, image_height });
        if (ImGui::IsItemHovered()) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            gpHoverPosX = (mouse_pos.x - image_pos.x) / 2;
            gpHoverPosY = (mouse_pos.y - image_pos.y) / 2;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && gpHoveredObject != -1) {
            // set panel state
            ScenePanel* panel = dynamic_cast<NativeWindow*>(_parent)->getImGuiPanel<ScenePanel>(SubPanels::scene);
            if (panel) {
                panel->setInspectObjectId(gpHoveredObject);
            }
        }
    }
    ImGui::End();
}

bool GamePanel::init()
{
    return true;
}

} // namespace editor