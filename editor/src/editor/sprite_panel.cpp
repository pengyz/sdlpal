#include "sprite_panel.h"
#include "editor/gui_convertor.h"
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

SpritePanel::SpritePanel(int width, int height, const std::string& title, bool visible, engine::PalEngine* engine)
    : Window(width, height, title, visible)
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
                    "currFrame", _pObject->wCurrentFrameNum, nullptr, std::function<bool(WORD)>([this](WORD val) {
                        return val >= 0 && val < _pObject->nSpriteFrames;
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

inline BYTE calcShadowColor(BYTE bSourceColor)
{
    return ((bSourceColor & 0xF0) | ((bSourceColor & 0x0F) >> 1));
}

INT RLEBlitToSurfaceWithShadow(LPCBITMAPRLE lpBitmapRLE, SDL_Surface* lpDstSurface, PAL_POS pos, BOOL bShadow)
{
    UINT i, j, k, sx;
    INT x, y;
    UINT uiLen = 0;
    UINT uiWidth = 0;
    UINT uiHeight = 0;
    UINT uiSrcX = 0;
    BYTE T;
    INT dx = PAL_X(pos);
    INT dy = PAL_Y(pos);
    LPBYTE p;

    //
    // Check for NULL pointer.
    //
    if (lpBitmapRLE == NULL || lpDstSurface == NULL) {
        return -1;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 && lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00) {
        lpBitmapRLE += 4;
    }

    //
    // Get the width and height of the bitmap.
    //
    uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
    uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

    //
    // Check whether bitmap intersects the surface.
    //
    if (uiWidth + dx <= 0 || dx >= lpDstSurface->w || uiHeight + dy <= 0 || dy >= lpDstSurface->h) {
        goto end;
    }

    //
    // Calculate the total length of the bitmap.
    // The bitmap is 8-bpp, each pixel will use 1 byte.
    //
    uiLen = uiWidth * uiHeight;

    //
    // Start decoding and blitting the bitmap.
    //
    lpBitmapRLE += 4;
    for (i = 0; i < uiLen;) {
        T = *lpBitmapRLE++;
        if ((T & 0x80) && T <= 0x80 + uiWidth) {
            i += T - 0x80;
            uiSrcX += T - 0x80;
            if (uiSrcX >= uiWidth) {
                uiSrcX -= uiWidth;
                dy++;
            }
        } else {
            //
            // Prepare coordinates.
            //
            j = 0;
            sx = uiSrcX;
            x = dx + uiSrcX;
            y = dy;

            //
            // Skip the points which are out of the surface.
            //
            if (y < 0) {
                j += -y * uiWidth;
                y = 0;
            } else if (y >= lpDstSurface->h) {
                goto end; // No more pixels needed, break out
            }

            while (j < T) {
                //
                // Skip the points which are out of the surface.
                //
                if (x < 0) {
                    j += -x;
                    if (j >= T)
                        break;
                    sx += -x;
                    x = 0;
                } else if (x >= lpDstSurface->w) {
                    j += uiWidth - sx;
                    x -= sx;
                    sx = 0;
                    y++;
                    if (y >= lpDstSurface->h) {
                        goto end; // No more pixels needed, break out
                    }
                    continue;
                }

                //
                // Put the pixels in row onto the surface
                //
                k = T - j;
                if (lpDstSurface->w - x < k)
                    k = lpDstSurface->w - x;
                if (uiWidth - sx < k)
                    k = uiWidth - sx;
                sx += k;
                p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
                if (bShadow) {
                    j += k;
                    for (; k != 0; k--) {
                        p[x] = calcShadowColor(p[x]);
                        x++;
                    }
                } else {
                    for (; k != 0; k--) {
                        p[x] = lpBitmapRLE[j];
                        j++;
                        x++;
                    }
                }

                if (sx >= uiWidth) {
                    sx -= uiWidth;
                    x -= uiWidth;
                    y++;
                    if (y >= lpDstSurface->h) {
                        goto end; // No more pixels needed, break out
                    }
                }
            }
            lpBitmapRLE += T;
            i += T;
            uiSrcX += T;
            while (uiSrcX >= uiWidth) {
                uiSrcX -= uiWidth;
                dy++;
            }
        }
    }

end:
    //
    // Success
    //
    return 0;
}

void SpritePanel::updateSpriteTexture(WORD eventObjectId, engine::LPEVENTOBJECT pObject)
{
    int spriteWidth = 0, spriteHeight = 0;
    if (model.spriteObjectId == eventObjectId && model.texture) {
        // already loaded.
        return;
    }
    if (model.spriteObjectId != eventObjectId) {
        // change texture
        if (model.texture) {
            if (SDL_QueryTexture(model.texture, nullptr, nullptr, &spriteWidth, &spriteHeight)) {
                return;
            }
            // update texture size
            model.texture_size.x = (float)spriteWidth;
            model.texture_size.y = (float)spriteHeight;
        }
        model.spriteObjectId = eventObjectId;
    }
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
    if ((spriteWidth && abs(spriteWidth - rle_width * model.texture_scale) > 0.01) || (spriteHeight && abs(spriteHeight - rle_height * model.texture_scale) > 0.01)) {
        SDL_DestroyTexture(model.texture);
        model.texture = nullptr;
    }
    if (!model.texture) {
        model.texture_size.x = (float)rle_width * model.texture_scale;
        model.texture_size.y = (float)rle_height * model.texture_scale;
        model.texture = SDL_CreateTexture(_engine->getRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
            model.texture_size.x, model.texture_size.y);
    }
    SDL_Surface* surface = NULL;
    if (SDL_LockTextureToSurface(model.texture, nullptr, &surface)) {
        if (model.texture) {
            SDL_DestroyTexture(model.texture);
            model.texture = nullptr;
        }
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
    int ret = RLEBlitToSurfaceWithShadow(lpFrame, surface, PAL_XY(posX, posY), false);
    SDL_UnlockTexture(model.texture);
    SDL_FreePalette(_palette);
    if (ret) {
        if (model.texture) {
            SDL_DestroyTexture(model.texture);
            model.texture = nullptr;
        }
    }
}

} // namespace editor