#include "enemy_panel.h"
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "common.h"
#include "editor_meta.h"
#include "engine/pal_common.h"
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
    freeSprites();
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
        addPropertyReadonly("enemyID", pEnemy->wEnemyID);
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
}

void EnemyPanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(_title.c_str())) {
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("队伍列表")) {
            if (ImGui::BeginListBox("##EnemyList", { -FLT_MIN, -FLT_MIN })) {
                engine::GAMEDATA& data = _engine->getGlobals()->getGameData();
                char buf[128];
                for (int i = 0; i < data.nEnemyTeam; i++) {
                    sprintf(buf, "team %d", i);
                    if (ImGui::TreeNodeEx(buf)) {
                        // load current team sprites
                        loadSprites(&data.lprgEnemyTeam[i]);
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
                            int flags = ImGuiTreeNodeFlags_AllowOverlap;
                            bool ret = ImGui::TreeNodeEx(buf, flags);
                            auto pEnemy = &obj_enemy;
                            // update image
                            if (model.sprites.count(pEnemy->wEnemyID) && model.sprites[pEnemy->wEnemyID].texture) {
                                ImGui::SameLine();
                                SpriteDetail& detail = model.sprites[pEnemy->wEnemyID];
                                ImGui::Image(detail.texture, ImVec2(detail.width, detail.height));
                            }
                            if (ret) {
                                drawEnemyPropertyTable(j, &obj_enemy);
                                // detail
                                sprintf(buf, "battle detail##%d,%d,%d", j, obj_enemy.wEnemyID, i);
                                ImGui::Indent();
                                if (ImGui::TreeNodeEx(buf)) {
                                    // draw property
                                    engine::LPENEMY enemyDetails = &data.lprgEnemy[obj_enemy.wEnemyID];
                                    drawEnemyBattlePropertyTable(j, enemyDetails);
                                    ImGui::TreePop();
                                }
                                ImGui::Unindent();
                                ImGui::TreePop();
                            }
                            ImGui::Unindent();
                        }
                        ImGui::TreePop();
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
    return true;
}

void EnemyPanel::freeSprites()
{
    for (auto& pair : model.sprites) {
        SpriteDetail& detail = pair.second;
        if (detail.texture) {
            SDL_DestroyTexture(detail.texture);
            detail.texture = nullptr;
        }
        if (detail.lpSprite) {
            free(detail.lpSprite);
            detail.lpSprite = nullptr;
        }
    }
    model.sprites.clear();
}

void EnemyPanel::loadSprites(engine::LPENEMYTEAM enemyTeam)
{
    FILE* fp = UTIL_OpenRequiredFile("abc.mkf");
    engine::GAMEDATA& data = _engine->getGlobals()->getGameData();
    auto _palette = SDL_AllocPalette(256);
    SDL_Color* p = PAL_GetPalette(0, false);
    for (int i = 0; i < MAX_ENEMIES_IN_TEAM; i++) {
        DWORD wEnemyObjectID = enemyTeam->rgwEnemy[i];
        if (wEnemyObjectID == 0xFFFF || wEnemyObjectID == 0) {
            continue;
        }
        WORD wEnemyID = data.rgObject[wEnemyObjectID].enemy.wEnemyID;
        if (model.sprites.count(wEnemyID)) {
            // already exist
            continue;
        }

        // load sprite
        int l = PAL_MKFGetDecompressedSize(wEnemyID, fp);
        if (l <= 0) {
            continue;
        }
        SpriteDetail detail;
        detail.lpSprite = static_cast<LPSPRITE>(UTIL_calloc(l, 1));
        PAL_MKFDecompressChunk(detail.lpSprite, l, wEnemyID, fp);
        // get texture
        LPCBITMAPRLE lpFrame = PAL_SpriteGetFrame(detail.lpSprite, 0);
        detail.width = PAL_RLEGetWidth(lpFrame);
        detail.height = PAL_RLEGetHeight(lpFrame);
        SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, detail.width, detail.height, 8, 0, 0, 0, 0);
        if (!surface) {
            continue;
        }
        SDL_SetPaletteColors(_palette, p, 0, 256);
        SDL_SetSurfacePalette(surface, _palette);

        //
        // HACKHACK: need to invalidate _screen->map otherwise the palette
        // would not be effective during blit
        //
        SDL_SetSurfaceColorMod(surface, 0, 0, 0);
        SDL_SetSurfaceColorMod(surface, 0xFF, 0xFF, 0xFF);
        if (!engine::RLEBlitToSurfaceWithShadow(lpFrame, surface, PAL_XY(0, 0), false, false)) {
            detail.texture = SDL_CreateTextureFromSurface(_engine->getPalRenderer()->getRenderer(), surface);
            model.sprites[wEnemyID] = detail;
        }
        SDL_FreeSurface(surface);
    }
    SDL_FreePalette(_palette);
    fclose(fp);
}

} // namespace editor