#include "enemy_panel.h"
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

EnemyPanel::EnemyPanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(parent, width, height, title, visible)
    , _engine(engine)
{
}

EnemyPanel::~EnemyPanel()
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

// void showSceneList(ScenePanelModel& model, engine::PalGlobals* globals, engine::PalResources* resources)
// {
//     char preview_value[128] = { 0 };
//     genMapName(preview_value, &globals->getGameData().rgScene[model.item_current_idx], model.item_current_idx);
//     ImGui::Text("地图选择:");
//     ImGui::SameLine();
//     if (ImGui::BeginCombo("##scenes", preview_value, ImGuiComboFlags_HeightLarge | ImGuiComboFlags_WidthFitPreview)) {
//         for (int n = 0; n < IM_ARRAYSIZE(globals->getGameData().rgScene); n++) {
//             engine::LPSCENE pScene = &globals->getGameData().rgScene[n];
//             if (!pScene->wMapNum)
//                 break;
//             const bool is_selected = (model.item_current_idx == n);
//             char buf[128];
//             genMapName(buf, pScene, n);
//             if (ImGui::Selectable(buf, is_selected)) {
//                 if (n != model.item_current_idx) {
//                     model.item_current_idx = n;
//                     // load scene
//                     resources->setLoadFlags(engine::kLoadScene | engine::kLoadPlayerSprite);
//                     globals->getEnteringScene() = TRUE;
//                     globals->getNumScene() = model.item_current_idx + 1;
//                 }
//             }

//             // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
//             if (is_selected) {
//                 ImGui::SetItemDefaultFocus();
//             }
//         }
//         ImGui::EndCombo();
//     }
//     ImGui::SameLine();
//     // edit name
//     if (ImGui::Button("修改地图名##editMapName")) {
//         // popup editor
//         ImGui::OpenPopup("mapName_popup");
//     }
//     if (ImGui::BeginPopup("mapName_popup")) {
//         char mapName[64];
//         memset(mapName, 0, IM_ARRAYSIZE(mapName));
//         const char* name = EditorMeta::get().getMapName(model.item_current_idx);
//         if (strlen(name)) {
//             strcpy(mapName, name);
//         }
//         ImGui::Text("地图名:");
//         ImGui::SameLine();
//         if (ImGui::InputText("##map name", mapName, IM_ARRAYSIZE(mapName), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
//             engine::PalLog::get().addLog(engine::LogLevel::info, "new map name: %s", mapName);
//             EditorMeta::get().setMapName(model.item_current_idx, mapName);
//             EditorMeta::get().sync();
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::EndPopup();
//     }
// }

void EnemyPanel::drawObjectPropertyTable(WORD wEventObjectID, engine::LPEVENTOBJECT pObject)
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

void EnemyPanel::drawEnemyPropertyTable(int idx, engine::OBJECT_ENEMY* pEnemy)
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
    sprintf(buf, "#objectPropertyTable%d", idx);
    if (ImGui::BeginTable(buf, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("属性");
        ImGui::TableSetupColumn("值");
        ImGui::TableHeadersRow();
        addPropertyReadonly("enemyID", pEnemy->wEnemyID, std::function<void(decltype(pEnemy->wEnemyID))>([](decltype(pEnemy->wEnemyID) enemyID) {
            if (enemyID) {
                ImGui::SameLine();
                char buf[64];
                sprintf(buf, "查看##%d", enemyID);
                if (ImGui::Button(buf)) {
                    // show script data
                    engine::PalLog::get().addLog(engine::LogLevel::info, "enemyID: %d", enemyID);
                }

                // update image
                // ImGui::Image(ImTextureID user_texture_id, const ImVec2 &image_size);
            }
        }));
        addPropertyEditableNoCheck("resistanceToSorcery", pEnemy->wResistanceToSorcery);
        addPropertyEditableNoCheck("scriptOnTurnStart", pEnemy->wScriptOnTurnStart);
        addPropertyEditableNoCheck("scriptOnBattleEnd", pEnemy->wScriptOnBattleEnd);
        addPropertyReadonly("scriptOnReady", pEnemy->wScriptOnReady, std::function<void(decltype(pEnemy->wScriptOnReady))>([this, idx, pEnemy](decltype(pEnemy->wScriptOnReady) wScriptNum) {
            // show button
            if (wScriptNum) {
                ImGui::SameLine();
                char buf[64];
                sprintf(buf, "预览##%d", wScriptNum);
                if (ImGui::Button(buf)) {
                    // show preview
                    // this->_spritePanel->openWindow(wEventObjectID, pObject);
                }
            }
        }));
        // // add error popups
        addErrorPopups("Err_ValueOutOfRange", []() {
            ImGui::Text("value out of range !");
        });

        invokeErrorPopups();
    }
    ImGui::EndTable();
    // render sprite panel
    _spritePanel->render();
}

void EnemyPanel::drawEnemyBattlePropertyTable(int idx, engine::LPENEMY pEnemy)
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
#define ADD_EDIT_PROPERTY(name) addPropertyEditableNoCheck(#name, pEnemy->name);
#define ADD_EDIT_PROPERTY_READONLY(name) addPropertyReadonly(#name, pEnemy->name);
#define ADD_EDIT_PROPERTY_ARRAY(name, max)                \
    for (int i = 0; i < max; i++) {                       \
        char buf[128];                                    \
        sprintf(buf, "%s%d", #name, i);                   \
        addPropertyEditableNoCheck(buf, pEnemy->name[i]); \
    }
    char buf[128];
    sprintf(buf, "#enemyBattlePropertyTable%d", idx);
    if (ImGui::BeginTable(buf, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("属性");
        ImGui::TableSetupColumn("值");
        ImGui::TableHeadersRow();
        ADD_EDIT_PROPERTY_READONLY(wIdleFrames);
        ADD_EDIT_PROPERTY_READONLY(wMagicFrames);
        ADD_EDIT_PROPERTY_READONLY(wAttackFrames);
        ADD_EDIT_PROPERTY(wIdleAnimSpeed);
        ADD_EDIT_PROPERTY(wActWaitFrames); // FIXME: ???
        ADD_EDIT_PROPERTY(wYPosOffset);
        ADD_EDIT_PROPERTY(wAttackSound); // sound played when this enemy uses normal attack
        ADD_EDIT_PROPERTY(wActionSound); // FIXME: ???
        ADD_EDIT_PROPERTY(wMagicSound); // sound played when this enemy uses magic
        ADD_EDIT_PROPERTY(wDeathSound); // sound played when this enemy dies
        ADD_EDIT_PROPERTY(wCallSound); // sound played when entering the battle
        ADD_EDIT_PROPERTY(wHealth); // total HP of the enemy
        ADD_EDIT_PROPERTY(wExp); // How many EXPs we'll get for beating this enemy
        ADD_EDIT_PROPERTY(wCash); // how many cashes we'll get for beating this enemy
        ADD_EDIT_PROPERTY(wLevel); // this enemy's level
        ADD_EDIT_PROPERTY(wMagic); // this enemy's magic number
        ADD_EDIT_PROPERTY(wMagicRate); // chance for this enemy to use magic
        ADD_EDIT_PROPERTY(wAttackEquivItem); // equivalence item of this enemy's normal attack
        ADD_EDIT_PROPERTY(wAttackEquivItemRate); // chance for equivalence item
        ADD_EDIT_PROPERTY(wStealItem); // which item we'll get when stealing from this enemy
        ADD_EDIT_PROPERTY(nStealItem); // total amount of the items which can be stolen
        ADD_EDIT_PROPERTY(wAttackStrength); // normal attack strength
        ADD_EDIT_PROPERTY(wMagicStrength); // magical attack strength
        ADD_EDIT_PROPERTY(wDefense); // resistance to all kinds of attacking
        ADD_EDIT_PROPERTY(wDexterity); // dexterity
        ADD_EDIT_PROPERTY(wFleeRate); // chance for successful fleeing
        ADD_EDIT_PROPERTY(wPoisonResistance); // resistance to poison
        ADD_EDIT_PROPERTY_ARRAY(wElemResistance, NUM_MAGIC_ELEMENTAL); // resistance to elemental magics
        ADD_EDIT_PROPERTY(wPhysicalResistance); // resistance to physical attack
        ADD_EDIT_PROPERTY(wDualMove); // whether this enemy can do dual move or not
        ADD_EDIT_PROPERTY(wCollectValue); // value for collecting this enemy for items
        // // add error popups
        addErrorPopups("Err_ValueOutOfRange", []() {
            ImGui::Text("value out of range !");
        });

        invokeErrorPopups();
    }
    ImGui::EndTable();
    // render sprite panel
    _spritePanel->render();
}

void EnemyPanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str(), nullptr)) {
        if (ImGui::CollapsingHeader("敌人队伍列表")) {
            if (ImGui::BeginListBox("##EnemyList", { -FLT_MIN, -FLT_MIN })) {
                engine::GAMEDATA& data = _engine->getGlobals()->getGameData();
                char buf[128];
                for (int i = 0; i < data.nEnemyTeam; i++) {
                    sprintf(buf, "team %d", i);
                    if (ImGui::CollapsingHeader(buf, 0)) {
                        for (int j = 0; j < MAX_ENEMIES_IN_TEAM; j++) {
                            DWORD wEnemyObjectID = data.lprgEnemyTeam[i].rgwEnemy[j];
                            if (wEnemyObjectID == 0xFFFF || wEnemyObjectID == 0) {
                                continue;
                            }
                            engine::OBJECT_ENEMY& obj_enemy = data.rgObject[wEnemyObjectID].enemy;
                            // create item
                            ImGui::Indent();
                            char buf[128];
                            sprintf(buf, "%d enemyId: %d##%d", j, obj_enemy.wEnemyID, i);
                            int flags = 0;
                            if (model.selected_enemy_id == wEnemyObjectID) {
                                flags |= ImGuiTreeNodeFlags_Selected;
                            } else {
                                model.selected_enemy_id = wEnemyObjectID;
                            }
                            if (ImGui::CollapsingHeader(buf, flags)) {
                                drawEnemyPropertyTable(j, &obj_enemy);
                                // detail
                                sprintf(buf, "battle detail##%d,%d,%d", j, obj_enemy.wEnemyID, i);
                                ImGui::Indent();
                                if (ImGui::CollapsingHeader(buf)) {
                                    // draw property
                                    engine::LPENEMY enemyDetails = &data.lprgEnemy[obj_enemy.wEnemyID];
                                    drawEnemyBattlePropertyTable(j, enemyDetails);
                                }
                                ImGui::Unindent();
                            }
                            ImGui::Unindent();
                        }
                    }
                }
                ImGui::EndListBox();
            }
        }
    }
    ImGui::End();
}

bool EnemyPanel::init()
{
    _spritePanel = new SpritePanel(_parent, 800, 600, "SpriteViewer", false, _engine);
    return true;
}

void EnemyPanel::updateEnemySprite(WORD wEnemyObjectID)
{
    // load sprite
    engine::GAMEDATA& data = _engine->getGlobals()->getGameData();
    engine::OBJECT_ENEMY& obj_enemy = data.rgObject[wEnemyObjectID].enemy;
    obj_enemy.wEnemyID;
    engine::LPENEMY enemyDetails = &data.lprgEnemy[obj_enemy.wEnemyID];
}

} // namespace editor