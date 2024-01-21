#include "scene_panel.h"
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "engine/pal_global.h"
#include "engine/pal_renderer.h"
#include "engine/pal_resources.h"
#include "global.h"
#include "gui_convertor.h"
#include "gui_template.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "palcommon.h"
#include "palette.h"
#include "sprite_panel.h"
#include "util.h"
#include <SDL.h>
#include <cfloat>
#include <string>

namespace editor {

ScenePanel::ScenePanel(int width, int height, const std::string& title, bool visible, engine::PalGlobals* globals, engine::PalResources* resources, engine::PalRenderer* renderer)
    : Window(width, height, title, visible)
    , _globals(globals)
    , _resources(resources)
    , _renderer(renderer)
{
}

ScenePanel::~ScenePanel()
{
    if (_spritePanel) {
        delete _spritePanel;
        _spritePanel = nullptr;
    }
}

void showSceneList(ScenePanelModel& model, engine::PalGlobals* globals, engine::PalResources* resources)
{
    char preview_value[512] = { 0 };
    sprintf(preview_value, "%.3d mapNum: %d", model.item_current_idx, globals->getGameData().rgScene[model.item_current_idx].wMapNum);
    if (ImGui::BeginCombo("##scenes", preview_value, ImGuiComboFlags_HeightLarge | ImGuiComboFlags_WidthFitPreview)) {
        for (int n = 0; n < IM_ARRAYSIZE(globals->getGameData().rgScene); n++) {
            engine::LPSCENE pScene = &globals->getGameData().rgScene[n];
            if (!pScene->wMapNum)
                break;
            const bool is_selected = (model.item_current_idx == n);
            char buf[128];
            sprintf(buf, "%.3d mapNum: %d", n, pScene->wMapNum);
            if (ImGui::Selectable(buf, is_selected)) {
                if (n != model.item_current_idx) {
                    model.item_current_idx = n;
                    // load scene
                    resources->setLoadFlags(engine::kLoadScene | engine::kLoadPlayerSprite);
                    globals->getEnteringScene() = TRUE;
                    globals->getNumScene() = model.item_current_idx + 1;
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void ScenePanel::drawObjectPropertyTable(int n, WORD wEventObjectID, engine::LPEVENTOBJECT pObject)
{
    // error popups
    std::map<std::string, std::function<void()>> _errorPopups;
    auto invokeErrorPopups = [&_errorPopups]() {
        for (const auto& pair : _errorPopups) {
            if (ImGui::BeginPopup(pair.first.c_str())) {
                pair.second();
                ImGui::EndPopup();
            }
        }
    };

    auto addErrorPopups = [&_errorPopups](const char* key, std::function<void()> callback) {
        _errorPopups[key] = callback;
    };

    char buf[128];
    sprintf(buf, "#objectPropertyTable%d", n);
    if (ImGui::BeginTable(buf, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("属性");
        ImGui::TableSetupColumn("值");
        ImGui::TableHeadersRow();
        addObjectPropertyEditable("vanishTime", pObject->sVanishTime);
        addObjectPropertyEditableNoCheck("x", pObject->x);
        addObjectPropertyEditableNoCheck("y", pObject->y);
        addObjectPropertyEditable("layer", pObject->sLayer);
        addObjectPropertyReadonly("triggerScript", pObject->wTriggerScript);
        addObjectPropertyEditable("autoScript", pObject->wAutoScript);
        addObjectPropertySelectable(obj_state_convertor, "state", pObject->sState);
        addObjectPropertySelectable(obj_trigger_mode_convertor, "triggerMode", pObject->wTriggerMode);
        addObjectPropertyReadonly("spriteNum", pObject->wSpriteNum, std::function<void(decltype(pObject->wSpriteNum))>([this, wEventObjectID, pObject](decltype(pObject->wSpriteNum) wScriptNum) {
            // show button
            if (wScriptNum) {
                ImGui::SameLine();
                char buf[64];
                sprintf(buf, "预览##%d", wScriptNum);
                if (ImGui::Button(buf)) {
                    // show preview
                    this->_spritePanel->openWindow(wEventObjectID, pObject);
                }
            }
        }));
        addObjectPropertyReadonly("spriteFrames", pObject->nSpriteFrames);
        addObjectPropertySelectable(obj_direction_converter, "direction", pObject->wDirection);
        addObjectPropertyEditable(
            "currFrame", pObject->wCurrentFrameNum, nullptr, std::function<bool(WORD)>([pObject](WORD val) {
                return val >= 0 && val < pObject->nSpriteFrames;
            }),
            "Err_ValueOutOfRange");
        addObjectPropertyEditable("scriptIdleFrame", pObject->nScriptIdleFrame);
        addObjectPropertyEditable("spritePtrOffset", pObject->wSpritePtrOffset);
        addObjectPropertyEditable("spriteFramesAuto", pObject->nSpriteFramesAuto);
        addObjectPropertyEditable("scriptIdleFrameCountAuto", pObject->wScriptIdleFrameCountAuto);

        // add error popups
        addErrorPopups("Err_ValueOutOfRange", []() {
            ImGui::Text("value out of range !");
        });

        invokeErrorPopups();
        ImGui::EndTable();
    }
    // render sprite panel
    _spritePanel->render();
}

void ScenePanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::LabelText("##title", "%s", "场景列表");
        // show scene list
        showSceneList(model, _globals, _resources);
        // show scene details
        engine::SCENE* pScene = &_globals->getGameData().rgScene[model.item_current_idx];
        ImGui::LabelText("##entryScript", "enterScript: %.4x", pScene->wScriptOnEnter);
        ImGui::LabelText("##teleport", "teleportScript: %.4x", pScene->wScriptOnTeleport);
        if (ImGui::CollapsingHeader("对象列表")) {
            if (ImGui::BeginListBox("##ObjectList", { -FLT_MIN, -FLT_MIN })) {
                WORD beginObjectIndex = _globals->getGameData().rgScene[_globals->getNumScene() - 1].wEventObjectIndex + 1;
                WORD endObjectIndex = _globals->getGameData().rgScene[_globals->getNumScene()].wEventObjectIndex;
                for (WORD wEventObjectID = beginObjectIndex; wEventObjectID <= endObjectIndex; wEventObjectID++) {
                    engine::LPEVENTOBJECT pObject = &_globals->getGameData().lprgEventObject[wEventObjectID - 1];
                    WORD n = wEventObjectID - beginObjectIndex;
                    const bool is_selected = (model.object_selected_idx == n);
                    char buf[128];
                    sprintf(buf, "objectId: %d", wEventObjectID);
                    int flags = 0;
                    if (model.object_selected_idx == n) {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    if (ImGui::CollapsingHeader(buf, flags)) {
                        if (model.object_selected_idx != n) {
                            if (model.object_selected_idx != -1) {
                                // close previous open item
                                sprintf(buf, "objectId: %d", model.object_selected_idx + beginObjectIndex);
                                ImGui::TreeNodeSetOpen(ImGui::GetID(buf), false);
                                _spritePanel->closeWindow();
                            }
                            model.object_selected_idx = n;
                        }
                        if (model.object_selected_idx == n) {
                            // draw object property table
                            drawObjectPropertyTable(n, wEventObjectID, pObject);
                        }
                    }
                }
                ImGui::EndListBox();
            }
        }
        ImGui::End();
    }
}

bool ScenePanel::init()
{
    _spritePanel = new SpritePanel(800, 600, "SpriteViewer", false, _globals, _resources, _renderer);
    return true;
}

} // namespace editor