#include "scene_panel.h"
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "engine/pal_engine.h"
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

ScenePanel::ScenePanel(int width, int height, const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(width, height, title, visible)
    , _engine(engine)
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
    char preview_value[128] = { 0 };
    sprintf(preview_value, "%.3d mapNum: %d", model.item_current_idx, globals->getGameData().rgScene[model.item_current_idx].wMapNum);
    ImGui::Text("地图选择:");
    ImGui::SameLine();
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
        addPropertyEditable("vanishTime", pObject->sVanishTime);
        addPropertyEditableNoCheck("x", pObject->x);
        addPropertyEditableNoCheck("y", pObject->y);
        addPropertyEditable("layer", pObject->sLayer);
        addPropertyReadonly("triggerScript", pObject->wTriggerScript);
        addPropertyEditable("autoScript", pObject->wAutoScript);
        addPropertySelectable(obj_state_convertor, "state", pObject->sState);
        addPropertySelectable(obj_trigger_mode_convertor, "triggerMode", pObject->wTriggerMode);
        addPropertyReadonly("spriteNum", pObject->wSpriteNum, std::function<void(decltype(pObject->wSpriteNum))>([this, wEventObjectID, pObject](decltype(pObject->wSpriteNum) wScriptNum) {
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
        addPropertyReadonly("spriteFrames", pObject->nSpriteFrames);
        addPropertySelectable(obj_direction_converter, "direction", pObject->wDirection);
        addPropertyEditable(
            "currFrame", pObject->wCurrentFrameNum, nullptr, std::function<bool(WORD)>([pObject](WORD val) {
                return val >= 0 && val < pObject->nSpriteFrames;
            }),
            "Err_ValueOutOfRange");
        addPropertyEditable("scriptIdleFrame", pObject->nScriptIdleFrame);
        addPropertyEditable("spritePtrOffset", pObject->wSpritePtrOffset);
        addPropertyEditable("spriteFramesAuto", pObject->nSpriteFramesAuto);
        addPropertyEditable("scriptIdleFrameCountAuto", pObject->wScriptIdleFrameCountAuto);

        // add error popups
        addErrorPopups("Err_ValueOutOfRange", []() {
            ImGui::Text("value out of range !");
        });

        invokeErrorPopups();
    }
    ImGui::EndTable();
    // render sprite panel
    _spritePanel->render();
}

void ScenePanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::LabelText("##title", "%s", "场景列表");
        // show scene list
        showSceneList(model, _engine->getGlobals(), _engine->getResources());
        ImGui::SameLine();
        ImGui::Dummy({ 5, 0 });
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Checkbox("绘制地图", &_engine->getDrawTileMap());
        ImGui::SameLine();
        ImGui::Dummy({ 5, 0 });
        ImGui::SameLine();
        ImGui::Checkbox("绘制对象", &_engine->getDrawSprite());
        ImGui::EndGroup();
        // show scene details
        engine::SCENE* pScene = &_engine->getGlobals()->getGameData().rgScene[model.item_current_idx];
        if (ImGui::CollapsingHeader("地图详情"), ImGuiTreeNodeFlags_DefaultOpen) {
            if (ImGui::BeginTable("##mapDetails", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
                // viewport
                ImGui::TableNextColumn();
                ImGui::Text("viewport");
                ImGui::TableNextColumn();
                int vals[2] = { PAL_X(_engine->getGlobals()->getViewport()), PAL_Y(_engine->getGlobals()->getViewport()) };
                if (ImGui::InputInt2("##viewport", vals)) {
                    _engine->getGlobals()->getViewport() = PAL_XY(vals[0], vals[1]);
                }
                addPropertyReadonly("enterScript", pScene->wScriptOnEnter, std::function<void(decltype(pScene->wScriptOnEnter))>([](decltype(pScene->wScriptOnEnter) entry) {
                    if (entry) {
                        ImGui::SameLine();
                        if (ImGui::Button("查看##enter")) {
                            // show script data
                            printf("preview for script %d\n", entry);
                        }
                    }
                }));
                addPropertyReadonly("teleportScript", pScene->wScriptOnTeleport, std::function<void(decltype(pScene->wScriptOnTeleport))>([](decltype(pScene->wScriptOnTeleport) entry) {
                    if (entry) {
                        ImGui::SameLine();
                        if (ImGui::Button("查看##teleport")) {
                            // show script data
                            printf("preview for script %d\n", entry);
                        }
                    }
                }));
            }
            ImGui::EndTable();
        }

        if (ImGui::CollapsingHeader("对象列表")) {
            if (ImGui::BeginListBox("##ObjectList", { -FLT_MIN, -FLT_MIN })) {
                WORD beginObjectIndex = _engine->getGlobals()->getGameData().rgScene[_engine->getGlobals()->getNumScene() - 1].wEventObjectIndex + 1;
                WORD endObjectIndex = _engine->getGlobals()->getGameData().rgScene[_engine->getGlobals()->getNumScene()].wEventObjectIndex;
                for (WORD wEventObjectID = beginObjectIndex; wEventObjectID <= endObjectIndex; wEventObjectID++) {
                    engine::LPEVENTOBJECT pObject = &_engine->getGlobals()->getGameData().lprgEventObject[wEventObjectID - 1];
                    WORD n = wEventObjectID - beginObjectIndex;
                    const bool is_selected = (model.object_selected_idx == n);
                    char buf[128];
                    sprintf(buf, "objectId: %d", wEventObjectID);
                    int flags = 0;
                    if (model.object_selected_idx == n) {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    ImGui::Indent();
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
                            // add button to focus object
                            char buf[128];
                            sprintf(buf, "聚焦##%d", wEventObjectID);
                            if (ImGui::Button(buf, { 100.f, 0.f })) {
                                _engine->getScene()->centerObject(wEventObjectID, pObject);
                            }
                        }
                    }
                    ImGui::Unindent();
                }
            }
            ImGui::EndListBox();
        }
    }
    ImGui::End();
}

bool ScenePanel::init()
{
    _spritePanel = new SpritePanel(800, 600, "SpriteViewer", false, _engine);
    return true;
}

} // namespace editor