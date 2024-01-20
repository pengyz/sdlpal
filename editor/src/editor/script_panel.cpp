#include "script_panel.h"
#include "engine/pal_global.h"
#include "engine/pal_resources.h"
#include "global.h"
#include "imgui.h"
#include "util.h"
#include <SDL.h>
#include <cfloat>
#include <string>

namespace editor {

ScriptPanel::ScriptPanel(int width, int height, const std::string& title, bool visible, engine::PalGlobals* globals, engine::PalResources* resources)
    : Window(width, height, title, visible)
    , _globals(globals)
    , _resources(resources)
{
}

ScriptPanel::~ScriptPanel() { }

void ScriptPanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::LabelText("##title", "%s", "脚本列表");
    }
    ImGui::End();
}

bool ScriptPanel::init() { return true; }

} // namespace editor