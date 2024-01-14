#include "scene_panel.h"
#include "global.h"
#include "imgui.h"
#include "res.h"
#include "util.h"
#include <SDL.h>
#include <cfloat>
#include <string>

extern GLOBALVARS* const gpGlobals;

namespace editor {

ScenePanel::ScenePanel(int width, int height, const std::string& title)
    : Window(width, height, title)
{
}

ScenePanel::~ScenePanel() { }

void ScenePanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::LabelText("##title", "%s", "场景列表");
        if (ImGui::BeginListBox("##scenes", { -FLT_MIN, -FLT_MIN })) {
            for (int n = 0; n < IM_ARRAYSIZE(gpGlobals->g.rgScene); n++) {
                LPSCENE pScene = &gpGlobals->g.rgScene[n];
                if (!pScene->wMapNum)
                    break;
                const bool is_selected = (model.item_current_idx == n);
                char buf[128];
                sprintf(buf, "%.3d mapNum: %d", n, pScene->wMapNum);
                if (ImGui::Selectable(buf, is_selected)) {
                    if (n != model.item_current_idx) {
                        model.item_current_idx = n;
                        // load scene
                        PAL_SetLoadFlags(kLoadScene | kLoadPlayerSprite);
                        gpGlobals->fEnteringScene = TRUE;
                        gpGlobals->wNumScene = model.item_current_idx + 1;
                    }
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }
    ImGui::End();
}

bool ScenePanel::init() { return true; }

} // namespace editor