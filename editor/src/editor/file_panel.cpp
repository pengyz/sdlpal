#include "file_panel.h"
#include "engine/pal_global.h"
#include "engine/pal_resources.h"
#include "global.h"
#include "imgui.h"
#include "util.h"
#include <SDL.h>
#include <cfloat>
#include <string>

namespace editor {

FilePanel::FilePanel(int width, int height, const std::string& title, bool visible, engine::PalGlobals* globals, engine::PalResources* resources)
    : Window(width, height, title, visible)
    , _globals(globals)
    , _resources(resources)
{
}

FilePanel::~FilePanel() { }

void FilePanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::LabelText("##title", "%s", "文件列表");
        // if (ImGui::BeginListBox("##scenes", { -FLT_MIN, -FLT_MIN })) {
        //     for (int n = 0; n < IM_ARRAYSIZE(_globals->getGameData().rgScene); n++) {
        //         engine::LPSCENE pScene = &_globals->getGameData().rgScene[n];
        //         if (!pScene->wMapNum)
        //             break;
        //         const bool is_selected = (model.item_current_idx == n);
        //         char buf[128];
        //         sprintf(buf, "%.3d mapNum: %d", n, pScene->wMapNum);
        //         if (ImGui::Selectable(buf, is_selected)) {
        //             if (n != model.item_current_idx) {
        //                 model.item_current_idx = n;
        //                 // load scene
        //                 _resources->setLoadFlags(engine::kLoadScene | engine::kLoadPlayerSprite);
        //                 _globals->getEnteringScene() = TRUE;
        //                 _globals->getNumScene() = model.item_current_idx + 1;
        //             }
        //         }

        //         // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        //         if (is_selected) {
        //             ImGui::SetItemDefaultFocus();
        //         }
        //     }
        //     ImGui::EndListBox();
        // }
    }
    ImGui::End();
}

bool FilePanel::init() { return true; }

} // namespace editor