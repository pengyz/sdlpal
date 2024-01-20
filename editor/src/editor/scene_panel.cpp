#include "scene_panel.h"
#include "engine/pal_global.h"
#include "engine/pal_resources.h"
#include "global.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "util.h"
#include <SDL.h>
#include <cfloat>
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>

namespace editor {

ScenePanel::ScenePanel(int width, int height, const std::string& title, bool visible, engine::PalGlobals* globals, engine::PalResources* resources)
    : Window(width, height, title, visible)
    , _globals(globals)
    , _resources(resources)
{
}

ScenePanel::~ScenePanel() { }

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

template <typename T, typename FN_getConvertTable>
void __addObjectPropertyEditable(FN_getConvertTable convertTable, const char* propName, T& value, std::function<bool(T)> cb = nullptr, const char* errorPopup = nullptr)
{
    std::pair<const char**, size_t> p = std::make_pair(nullptr, 0);
    if (convertTable) {
        p = convertTable();
    }
    ImGui::TableNextColumn();
    ImGui::Text("%s", propName);
    ImGui::TableNextColumn();
    char buf[128];
    sprintf(buf, "##%s", propName);
    bool ret = false;
    T val;
    if (p.first && p.second) {
        // create a selectable table
        const char* preview_value = nullptr;
        if (value >= 0 && value < p.second) {
            preview_value = p.first[value];
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(buf, preview_value)) {
            for (int i = 0; i < p.second; i++) {
                if (ImGui::Selectable(p.first[i], i == value)) {
                    // update value
                    value = i;
                }
            }
            ImGui::EndCombo();
        }
    } else {
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (std::is_integral<T>::value) {
            int tmp_val = value;
            ret = ImGui::InputInt(buf, &tmp_val, 1, 100, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
            val = tmp_val;
        } else if (std::is_floating_point<T>::value) {
            double tmp_val = value;
            ret = ImGui::InputDouble(buf, &tmp_val, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
            val = tmp_val;
        } else if (std::is_pointer<T>::value) {
        }
    }
    if (ret) {
        if (cb && !cb(val)) {
            if (errorPopup) {
                ImGui::OpenPopup(errorPopup);
            }
            return;
        }
        value = val;
    }
    ImGui::TableNextRow();
}

template <typename T>
void addObjectPropertyReadonly(const char* propName, T& value)
{
    ImGui::TableNextColumn();
    ImGui::Text("%s", propName);
    ImGui::TableNextColumn();
    ImGui::Text("%s", (std::stringstream() << value).str().c_str());
    ImGui::TableNextRow();
}

template <typename T>
void addObjectPropertyEditable(const char* propName, T& value, std::function<bool(T)> cb = nullptr, const char* errorPopup = nullptr)
{
    __addObjectPropertyEditable<T, std::pair<const char**, size_t>()>(nullptr, propName, value, cb, errorPopup);
}

template <typename T>
void addObjectPropertyEditableNoCheck(const char* propName, T& value)
{
    __addObjectPropertyEditable<T, std::pair<const char**, size_t>()>(nullptr, propName, value, std::function<bool(T)>(), nullptr);
}

template <typename T, typename FN_getConvertTable>
void addObjectPropertySelectable(FN_getConvertTable convertTable, const char* propName, T& value)
{
    __addObjectPropertyEditable<T, FN_getConvertTable>(convertTable, propName, value, nullptr, nullptr);
}

void drawObjectPropertyTable(int n, engine::LPEVENTOBJECT pObject)
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
        addObjectPropertyEditable("layer", pObject->sLayer, std::function<bool(SHORT)>(), nullptr);
        addObjectPropertyReadonly("triggerScript", pObject->wTriggerScript);
        addObjectPropertyEditable("autoScript", pObject->wAutoScript);
        addObjectPropertySelectable([]() {
            static const char* _names[] = {
                "kObjStateHidden",
                "kObjStateNormal",
                "kObjStateBlocker"
            };
            return std::make_pair(_names, sizeof(_names) / sizeof(_names[0]));
        },
            "state", pObject->sState);
        addObjectPropertySelectable([]() {
            static const char* _names[] = {
                "kTriggerNone",
                "kTriggerSearchNear",
                "kTriggerSearchNormal",
                "kTriggerSearchFar",
                "kTriggerTouchNear",
                "kTriggerTouchNormal",
                "kTriggerTouchFar",
                "kTriggerTouchFarther",
                "kTriggerTouchFarthest"
            };
            return std::make_pair(_names, sizeof(_names) / sizeof(_names[0]));
        },
            "triggerMode", pObject->wTriggerMode);
        addObjectPropertyReadonly("spriteNum", pObject->wSpriteNum);
        addObjectPropertyReadonly("spriteFrames", pObject->nSpriteFrames);
        addObjectPropertySelectable([]() {
            static const char* _names[] = {
                "kDirSouth",
                "kDirWest",
                "kDirNorth",
                "kDirEast",
            };
            return std::make_pair(_names, sizeof(_names) / sizeof(_names[0]));
        },
            "direction", pObject->wDirection);
        addObjectPropertyEditable(
            "currFrame", pObject->wCurrentFrameNum, std::function<bool(WORD)>([pObject](WORD val) {
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
                                // TODO: reset all ui state
                            }
                            model.object_selected_idx = n;
                        }
                        if (model.object_selected_idx == n) {
                            // draw object property table
                            drawObjectPropertyTable(n, pObject);
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
    return true;
}

} // namespace editor