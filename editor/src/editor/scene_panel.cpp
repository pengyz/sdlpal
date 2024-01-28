#include "scene_panel.h"
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "common.h"
#include "editor_meta.h"
#include "engine/pal_engine.h"
#include "engine/pal_log.h"
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
#include <cstdlib>
#include <iostream>
#include <string>

extern int gpHoveredObject;

namespace editor {

ScenePanel::ScenePanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(parent, width, height, title, visible)
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

static bool genMapName(char* buf, const engine::LPSCENE pScene, int n)
{
    const char* mapName = EditorMeta::get().getMapName(n);
    if (!strlen(mapName)) {
        sprintf(buf, "%.3d mapNum: %d", n, pScene->wMapNum);
        return false;
    } else {
        sprintf(buf, "%.3d %s mapNum: %.3d", n, mapName, pScene->wMapNum);
        return true;
    }
}

void showSceneList(ScenePanelModel& model, engine::PalGlobals* globals, engine::PalResources* resources)
{
    char preview_value[128] = { 0 };
    genMapName(preview_value, &globals->getGameData().rgScene[model.item_current_idx], model.item_current_idx);
    ImGui::Text("地图选择:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##scenes", preview_value, ImGuiComboFlags_HeightLarge | ImGuiComboFlags_WidthFitPreview)) {
        for (int n = 0; n < IM_ARRAYSIZE(globals->getGameData().rgScene); n++) {
            engine::LPSCENE pScene = &globals->getGameData().rgScene[n];
            if (!pScene->wMapNum)
                break;
            const bool is_selected = (model.item_current_idx == n);
            char buf[128];
            genMapName(buf, pScene, n);
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
    ImGui::SameLine();
    // edit name
    if (ImGui::Button("修改地图名##editMapName")) {
        // popup editor
        ImGui::OpenPopup("mapName_popup");
    }
    if (ImGui::BeginPopup("mapName_popup")) {
        char mapName[64];
        memset(mapName, 0, IM_ARRAYSIZE(mapName));
        const char* name = EditorMeta::get().getMapName(model.item_current_idx);
        if (strlen(name)) {
            strcpy(mapName, name);
        }
        ImGui::Text("地图名:");
        ImGui::SameLine();
        if (ImGui::InputText("##map name", mapName, IM_ARRAYSIZE(mapName), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            engine::PalLog::get().addLog(engine::LogLevel::info, "new map name: %s", mapName);
            EditorMeta::get().setMapName(model.item_current_idx, mapName);
            EditorMeta::get().sync();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ScenePanel::drawObjectPropertyTable(WORD wEventObjectID, engine::LPEVENTOBJECT pObject)
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
    sprintf(buf, "#objectPropertyTable%d", wEventObjectID);
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
    static const char* layer_tooltips[] = {
        "仅绘制规则图层，贴图为规则菱形",
        "仅绘制不规则图层，贴图为不规则形状",
        "绘制全部图层",
    };
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        ImGui::LabelText("##title", "%s", "场景列表");
        // show scene list
        showSceneList(model, _engine->getGlobals(), _engine->getResources());
        ImGui::SameLine();
        ImGui::Dummy({ 5, 0 });
        ImGui::BeginGroup();
        ImGui::Checkbox("绘制地图", &_engine->getDrawTileMap());
        ImGui::SameLine();
        ImGui::Dummy({ 5, 0 });
        ImGui::SameLine();
        ImGui::Checkbox("绘制对象", &_engine->getDrawSprite());
        ImGui::SameLine();
        ImGui::Checkbox("绘制图块线", &_engine->getDrawTileMapLines());
        ImGui::SameLine();
        auto getLayerName = [](int i) {
            char buf[64];
            if (i < engine::TileMapLayers_all) {
                sprintf(buf, "layer %d", i);
            } else {
                sprintf(buf, "layer all");
            }
            return std::string(buf);
        };
        ImGui::Dummy({ 5, 0 });
        ImGui::SameLine();
        ImGui::Text("图层");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##drawTileLayersList", getLayerName(_engine->getDrawTileLayers()).c_str(), ImGuiComboFlags_HeightLarge | ImGuiComboFlags_WidthFitPreview)) {
            for (int i = 0; i <= engine::TileMapLayers_all; i++) {
                auto name = getLayerName(i);
                if (ImGui::Selectable(name.c_str(), _engine->getDrawTileLayers() == i)) {
                    if (_engine->getDrawTileLayers() != i) {
                        _engine->getDrawTileLayers() = i;
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", layer_tooltips[i]);
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", layer_tooltips[_engine->getDrawTileLayers()]);
        }
        ImGui::EndGroup();
        // show scene details
        engine::SCENE* pScene
            = &_engine->getGlobals()->getGameData().rgScene[model.item_current_idx];
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

        if (ImGui::CollapsingHeader("对象列表")) {
            if (ImGui::BeginListBox("##ObjectList", { -FLT_MIN, -FLT_MIN })) {
                WORD beginObjectIndex = _engine->getGlobals()->getGameData().rgScene[_engine->getGlobals()->getNumScene() - 1].wEventObjectIndex + 1;
                WORD endObjectIndex = _engine->getGlobals()->getGameData().rgScene[_engine->getGlobals()->getNumScene()].wEventObjectIndex;
                if (model.object_id_to_open != -1) {
                    model.selected_object_id = model.object_id_to_open;
                }
                for (WORD wEventObjectID = beginObjectIndex; wEventObjectID <= endObjectIndex; wEventObjectID++) {
                    engine::LPEVENTOBJECT pObject = &_engine->getGlobals()->getGameData().lprgEventObject[wEventObjectID - 1];
                    char buf[128];
                    sprintf(buf, "objectId: %d", wEventObjectID);
                    int flags = 0;

                    if (model.object_id_to_open != -1) {
                        ImGui::TreeNodeSetOpen(ImGui::GetID(buf), wEventObjectID == model.object_id_to_open);
                        if (wEventObjectID == model.object_id_to_open) {
                            ImGui::ScrollToItem();
                        }
                    }
                    if (model.selected_object_id == wEventObjectID) {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    ImGui::Indent();
                    if (ImGui::CollapsingHeader(buf, flags)) {
                        if (model.selected_object_id != wEventObjectID) {
                            if (model.selected_object_id != -1) {
                                // close previous open item
                                sprintf(buf, "objectId: %d", model.selected_object_id);
                                ImGui::TreeNodeSetOpen(ImGui::GetID(buf), false);
                                _spritePanel->closeWindow();
                            }
                        } else {
                            // draw object property table
                            drawObjectPropertyTable(wEventObjectID, pObject);
                            // add button to focus object
                            char buf[128];
                            sprintf(buf, "聚焦##%d", wEventObjectID);
                            if (ImGui::Button(buf, { 100.f, 0.f })) {
                                _engine->getScene()->centerObject(wEventObjectID, pObject);
                                gpHoveredObject = wEventObjectID;
                            }
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("在场景面板中选中对象");
                            }
                        }
                    }
                    if (ImGui::IsItemToggledOpen()) {
                        model.selected_object_id = wEventObjectID;
                    }

                    ImGui::Unindent();
                }
                model.object_id_to_open = -1;
                ImGui::EndListBox();
            }
        }
    }
    ImGui::End();
}

bool ScenePanel::init()
{
    _spritePanel = new SpritePanel(_parent, 800, 600, "SpriteViewer", false, _engine);
    return true;
}

void ScenePanel::setInspectObjectId(int objectId)
{
    model.object_id_to_open = objectId;
}

} // namespace editor