#include "sprite_panel.h"
#include "3rd/SDL/include/SDL_render.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "editor/gui_convertor.h"
#include "engine/pal_common.h"
#include "engine/pal_engine.h"
#include "global.h"
#include "gui_template.h"
#include "imgui.h"
#include "palette.h"
#include "util.h"
#include <SDL.h>
#include <cfloat>
#include <string>

namespace editor {

SpritePanel::SpritePanel(Window* parent, int width, int height, const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(parent, width, height, title, visible)
    , _engine(engine)
{
}

SpritePanel::~SpritePanel() { }

void SpritePanel::render()
{
    auto value_changed = [this]() {
        this->model.value_changed = true;
    };
    if (model.sprite_review_open) {
        ImGui::SetNextWindowSize(ImVec2(_width, _height), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(_title.c_str(), &model.sprite_review_open)) {
            if (model.texture) {
                ImGui::Image((ImTextureID)model.texture, model.texture_size);
                if (model.prev_texture) {
                    // free previous texture
                    SDL_DestroyTexture(model.prev_texture);
                    model.prev_texture = nullptr;
                }
                ImGui::SameLine();
            }
            if (ImGui::BeginTable("##SpritePropertyTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("属性");
                ImGui::TableSetupColumn("值");
                ImGui::TableHeadersRow();
                // show sprite related properties
                addPropertyReadonly("spriteNum", _pObject->wSpriteNum);
                addPropertyReadonly("spriteFrames", _pObject->nSpriteFrames);
                addPropertySelectable(obj_direction_converter, "direction", _pObject->wDirection, value_changed);
                addPropertyEditable(
                    "currFrame", _pObject->wCurrentFrameNum, nullptr, std::function<bool(WORD)>([this, value_changed](WORD val) {
                        bool valid = val >= 0 && val < _pObject->nSpriteFrames;
                        return valid;
                    }));
            }
            ImGui::EndTable();
        }
        ImGui::End();
        if (model.value_changed) {
            model.value_changed = false;
            // update texture if property changed
            updateSpriteTexture(model.spriteObjectId, _pObject);
        }
    }
}

bool SpritePanel::init() { return true; }

void SpritePanel::updateSpriteTexture(WORD eventObjectId, engine::LPEVENTOBJECT pObject)
{
    int spriteWidth = 0, spriteHeight = 0;
    model.spriteObjectId = eventObjectId;
    LPSPRITE lpSprite = _engine->getResources()->getEventObjectSprite(eventObjectId);
    if (!lpSprite)
        return;
    LPCBITMAPRLE lpFrame = PAL_SpriteGetFrame(lpSprite,
        pObject->wDirection * pObject->nSpriteFrames + pObject->wCurrentFrameNum);
    if (!lpFrame)
        return;
    // frame got
    int rle_width = PAL_RLEGetWidth(lpFrame);
    int rle_height = PAL_RLEGetHeight(lpFrame);
    bool rebuild_texture = false;
    if (model.texture && (abs(model.texture_size.x - rle_width * model.texture_scale) > 0.01 || abs(model.texture_size.y - rle_height * model.texture_scale) > 0.01)) {
        // destroy and create texture
        rebuild_texture = true;
    }
    model.texture_size.x = (float)rle_width * model.texture_scale;
    model.texture_size.y = (float)rle_height * model.texture_scale;
    SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, model.texture_size.x, model.texture_size.y, 8, 0, 0, 0, 0);
    if (!surface) {
        return;
    }
    // set surface palette
    auto _palette = SDL_AllocPalette(256);
    SDL_Color* p = PAL_GetPalette(0, false);
    SDL_SetPaletteColors(_palette, p, 0, 256);
    SDL_SetSurfacePalette(surface, _palette);

    //
    // HACKHACK: need to invalidate _screen->map otherwise the palette
    // would not be effective during blit
    //
    SDL_SetSurfaceColorMod(surface, 0, 0, 0);
    SDL_SetSurfaceColorMod(surface, 0xFF, 0xFF, 0xFF);

    // center sprite in texture
    int posX = (model.texture_size.x - rle_width) / 2;
    int posY = (model.texture_size.y - rle_height) / 2;
    // draw RLE sprite to surface
    if (!engine::RLEBlitToSurfaceWithShadow(lpFrame, surface, PAL_XY(posX, posY), false, false)) {
        if (!model.texture || rebuild_texture) {
            model.prev_texture = model.texture;
            model.texture = SDL_CreateTextureFromSurface(_engine->getPalRenderer()->getRenderer(), surface);
        } else {
            // lock and copy
            void* texture_pixels = nullptr;
            int texture_pitch = 0;
            if (0 == SDL_LockTexture(model.texture, nullptr, &texture_pixels, &texture_pitch)) {
                uint8_t* src = (uint8_t*)surface->pixels;
                int w, h;
                SDL_QueryTexture(model.texture, nullptr, nullptr, &w, &h);
                uint8_t* dist = (uint8_t*)texture_pixels;
                for (int y = 0; y < h; y++, src += surface->pitch) {
                    memcpy(dist, src, w << 2);
                    dist += texture_pitch;
                }
                SDL_UnlockTexture(model.texture);
            }
        }
    }
    SDL_FreeSurface(surface);
    SDL_FreePalette(_palette);
}

} // namespace editor