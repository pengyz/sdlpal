#include "pal_scene.h"
#include "3rd/SDL/include/SDL_pixels.h"
#include "3rd/SDL/include/SDL_surface.h"
#include "common.h"
#include "global.h"
#include "pal_input.h"
#include "pal_resources.h"
#include "palcommon.h"
#include "palette.h"
#include "res.h"
#include <iostream>

extern bool gpHighlightHoverSprites;
extern int gpHoverPosX;
extern int gpHoverPosY;
extern int gpHighlightWidth;
extern int gpHighlightPaletteIndex;
int gpHoveredObject = -1;

namespace engine {

void PalScene::updateParty(PALINPUTSTATE* pInputState)
{
    int xSource, ySource, xTarget, yTarget, xOffset, yOffset, i;
    //
    // Has user pressed one of the arrow keys?
    //
    if (pInputState->dir != kDirUnknown) {
        xOffset = ((pInputState->dir == kDirWest || pInputState->dir == kDirSouth) ? -16 : 16);
        yOffset = ((pInputState->dir == kDirWest || pInputState->dir == kDirNorth) ? -8 : 8);

        xSource = PAL_X(_globals->getViewport()) + PAL_X(_globals->getPartyoffset());
        ySource = PAL_Y(_globals->getViewport()) + PAL_Y(_globals->getPartyoffset());

        xTarget = xSource + xOffset;
        yTarget = ySource + yOffset;

        _globals->getPartyDirection() = pInputState->dir;

        //
        // Check for obstacles on the destination location
        //
        // if (!checkObstacle(PAL_XY(xTarget, yTarget), TRUE, 0)) {
        //
        // Player will actually be moved. Store trail.
        //
        for (i = 3; i >= 0; i--) {
            _globals->getrgTrail()[i + 1] = _globals->getrgTrail()[i];
        }

        _globals->getrgTrail()[0].wDirection = pInputState->dir;
        _globals->getrgTrail()[0].x = xSource;
        _globals->getrgTrail()[0].y = ySource;

        //
        // Move the viewport
        //
        _globals->getViewport() = PAL_XY(PAL_X(_globals->getViewport()) + xOffset, PAL_Y(_globals->getViewport()) + yOffset);

        //
        // Update gestures
        //
        // updatePartyGestures(true);

        return; // don't go further
        // }
    }

    // updatePartyGestures(false);
}

void PalScene::updatePartyGestures(bool fWalking)
{
    int iStepFrameFollower = 0, iStepFrameLeader = 0;
    int i;

    if (fWalking) {
        //
        // Update the gesture for party leader
        //
        _iThisStepFrame = (_iThisStepFrame + 1) % 4;
        if (_iThisStepFrame & 1) {
            iStepFrameLeader = (_iThisStepFrame + 1) / 2;
            iStepFrameFollower = 3 - iStepFrameLeader;
        } else {
            iStepFrameLeader = 0;
            iStepFrameFollower = 0;
        }

        _globals->getrgParty()[0].x = PAL_X(_globals->getPartyoffset());
        _globals->getrgParty()[0].y = PAL_Y(_globals->getPartyoffset());

        if (_globals->getGameData().PlayerRoles.rgwWalkFrames[_globals->getrgParty()[0].wPlayerRole] == 4) {
            _globals->getrgParty()[0].wFrame = _globals->getPartyDirection() * 4 + _iThisStepFrame;
        } else {
            _globals->getrgParty()[0].wFrame = _globals->getPartyDirection() * 3 + iStepFrameLeader;
        }

        //
        // Update the gestures and positions for other party members
        //
        for (i = 1; i <= (short)_globals->getMaxPartyMemberIndex(); i++) {
            _globals->getrgParty()[i].x = _globals->getrgTrail()[1].x - PAL_X(_globals->getViewport());
            _globals->getrgParty()[i].y = _globals->getrgTrail()[1].y - PAL_Y(_globals->getViewport());

            if (i == 2) {
                _globals->getrgParty()[i].x += (_globals->getrgTrail()[1].wDirection == kDirEast || _globals->getrgTrail()[1].wDirection == kDirWest) ? -16 : 16;
                _globals->getrgParty()[i].y += 8;
            } else {
                _globals->getrgParty()[i].x += ((_globals->getrgTrail()[1].wDirection == kDirWest || _globals->getrgTrail()[1].wDirection == kDirSouth) ? 16 : -16);
                _globals->getrgParty()[i].y += ((_globals->getrgTrail()[1].wDirection == kDirWest || _globals->getrgTrail()[1].wDirection == kDirNorth) ? 8 : -8);
            }

            //
            // Adjust the position if there is obstacle
            //
            if (checkObstacle(PAL_XY(_globals->getrgParty()[i].x + PAL_X(_globals->getViewport()),
                                  _globals->getrgParty()[i].y + PAL_Y(_globals->getViewport())),
                    TRUE, 0)) {
                _globals->getrgParty()[i].x = _globals->getrgTrail()[1].x - PAL_X(_globals->getViewport());
                _globals->getrgParty()[i].y = _globals->getrgTrail()[1].y - PAL_Y(_globals->getViewport());
            }

            //
            // Update gesture for this party member
            //
            if (_globals->getGameData().PlayerRoles.rgwWalkFrames[_globals->getrgParty()[i].wPlayerRole] == 4) {
                _globals->getrgParty()[i].wFrame = _globals->getrgTrail()[2].wDirection * 4 + _iThisStepFrame;
            } else {
                _globals->getrgParty()[i].wFrame = _globals->getrgTrail()[2].wDirection * 3 + iStepFrameLeader;
            }
        }

        for (i = 1; i <= _globals->getFollower(); i++) {
            //
            // Update the position and gesture for the follower
            //
            _globals->getrgParty()[_globals->getMaxPartyMemberIndex() + i].x = _globals->getrgTrail()[2 + i].x - PAL_X(_globals->getViewport());
            _globals->getrgParty()[_globals->getMaxPartyMemberIndex() + i].y = _globals->getrgTrail()[2 + i].y - PAL_Y(_globals->getViewport());
            _globals->getrgParty()[_globals->getMaxPartyMemberIndex() + i].wFrame = _globals->getrgTrail()[2 + i].wDirection * 3 + iStepFrameFollower;
        }
    } else {
        //
        // Player is not moved. Use the "standing" gesture instead of "walking" one.
        //
        i = _globals->getGameData().PlayerRoles.rgwWalkFrames[_globals->getrgParty()[0].wPlayerRole];
        if (i == 0) {
            i = 3;
        }
        _globals->getrgParty()[0].wFrame = _globals->getPartyDirection() * i;

        for (i = 1; i <= (short)_globals->getMaxPartyMemberIndex(); i++) {
            int f = _globals->getGameData().PlayerRoles.rgwWalkFrames[_globals->getrgParty()[i].wPlayerRole];
            if (f == 0) {
                f = 3;
            }
            _globals->getrgParty()[i].wFrame = _globals->getrgTrail()[2].wDirection * f;
        }

        for (i = 1; i <= _globals->getFollower(); i++) {
            _globals->getrgParty()[_globals->getMaxPartyMemberIndex() + i].wFrame = _globals->getrgTrail()[2 + i].wDirection * 3;
        }

        _iThisStepFrame &= 2;
        _iThisStepFrame ^= 2;
    }
}

bool PalScene::checkObstacle(PAL_POS pos, bool fCheckEventObjects, WORD wSelfObject)
{
    int x, y, h, xr, yr;
    int blockX = PAL_X(_globals->getPartyoffset()) / 32, blockY = PAL_Y(_globals->getPartyoffset()) / 16;

    //
    // Check if the map tile at the specified position is blocking
    //
    x = PAL_X(pos) / 32;
    y = PAL_Y(pos) / 16;
    h = 0;

    //
    // Avoid walk out of range, look out of map
    //
    if (x < blockX || x >= 2048 || y < blockY || y >= 2048) {
        return TRUE;
    }

    xr = PAL_X(pos) % 32;
    yr = PAL_Y(pos) % 16;

    if (xr + yr * 2 >= 16) {
        if (xr + yr * 2 >= 48) {
            x++;
            y++;
        } else if (32 - xr + yr * 2 < 16) {
            x++;
        } else if (32 - xr + yr * 2 < 48) {
            h = 1;
        } else {
            y++;
        }
    }

    if (PAL_MapTileIsBlocked(x, y, h, _resources->getCurrentMap())) {
        return TRUE;
    }

    if (fCheckEventObjects) {
        //
        // Loop through all event objects in the current scene
        //
        int i;
        for (i = _globals->getGameData().rgScene[_globals->getNumScene() - 1].wEventObjectIndex;
             i < _globals->getGameData().rgScene[_globals->getNumScene()].wEventObjectIndex; i++) {
            LPEVENTOBJECT p = &(_globals->getGameData().lprgEventObject[i]);
            if (i == wSelfObject - 1) {
                //
                // Skip myself
                //
                continue;
            }

            //
            // Is this object a blocking one?
            //
            if (p->sState >= kObjStateBlocker) {
                //
                // Check for collision
                //
                if (abs(p->x - PAL_X(pos)) + abs(p->y - PAL_Y(pos)) * 2 < 16) {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

bool spriteHitTest(LPCBITMAPRLE lpBitmapRLE, SDL_Surface* lpDstSurface, PAL_POS pos, int mouseX, int mouseY)
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
    //
    // Check for NULL pointer.
    //
    if (lpBitmapRLE == NULL) {
        return false;
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
    // Check whether mouse is in bitmap rect.
    //
    int x1 = mouseX - dx;
    int y1 = mouseY - dy;
    if (x1 < 0 || x1 > uiWidth || y1 < 0 || y1 > uiHeight) {
        return false;
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
                if (y == mouseY) {
                    for (; k != 0; k--) {
                        if (x == mouseX) {
                            SDL_Color* p = PAL_GetPalette(0, false);
                            return p[lpBitmapRLE[j]].a == 0;
                        }
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
    return false;
}

void PalScene::drawSprites()
{
    int i, x, y, vy;

    _nSpriteToDraw = 0;

    //
    // Put all the sprites to be drawn into our array.
    //

    //
    // Players
    //
    for (i = 0; i <= (short)_globals->getMaxPartyMemberIndex() + _globals->getFollower(); i++) {
        LPCBITMAPRLE lpBitmap = _resources->spriteGetFrame(_resources->getPlayerSprite((BYTE)i), _globals->getrgParty()[i].wFrame);

        if (lpBitmap == NULL) {
            continue;
        }

        //
        // Add it to our array
        //
        addSpriteToDraw(i, lpBitmap,
            _globals->getrgParty()[i].x - PAL_RLEGetWidth(lpBitmap) / 2,
            _globals->getrgParty()[i].y + _globals->getLayer() + 10,
            _globals->getLayer() + 6);

        //
        // Calculate covering tiles on the map
        //
        calcCoverTiles(&_rgSpriteToDraw[_nSpriteToDraw - 1]);
    }

    //
    // Event Objects (Monsters/NPCs/others)
    //
    for (i = _globals->getGameData().rgScene[_globals->getNumScene() - 1].wEventObjectIndex;
         i < _globals->getGameData().rgScene[_globals->getNumScene()].wEventObjectIndex; i++) {
        LPCBITMAPRLE lpFrame;
        LPCSPRITE lpSprite;

        LPEVENTOBJECT lpEvtObj = &(_globals->getGameData().lprgEventObject[i]);

        int iFrame;

        if (lpEvtObj->sState == kObjStateHidden || lpEvtObj->sVanishTime > 0 || lpEvtObj->sState < 0) {
            continue;
        }

        //
        // Get the sprite
        //
        lpSprite = _resources->getEventObjectSprite((WORD)i + 1);
        if (lpSprite == NULL) {
            continue;
        }

        iFrame = lpEvtObj->wCurrentFrameNum;
        if (lpEvtObj->nSpriteFrames == 3) {
            //
            // walking character
            //
            if (iFrame == 2) {
                iFrame = 0;
            }

            if (iFrame == 3) {
                iFrame = 2;
            }
        }

        lpFrame = _resources->spriteGetFrame(lpSprite,
            lpEvtObj->wDirection * lpEvtObj->nSpriteFrames + iFrame);

        if (lpFrame == NULL) {
            continue;
        }

        //
        // Calculate the coordinate and check if outside the screen
        //
        x = (SHORT)lpEvtObj->x - PAL_X(_globals->getViewport());
        x -= PAL_RLEGetWidth(lpFrame) / 2;

        if (x >= SCENE_WIDTH || x < -(int)PAL_RLEGetWidth(lpFrame)) {
            //
            // outside the screen; skip it
            //
            continue;
        }

        y = (SHORT)lpEvtObj->y - PAL_Y(_globals->getViewport());
        y += lpEvtObj->sLayer * 8 + 9;

        vy = y - PAL_RLEGetHeight(lpFrame) - lpEvtObj->sLayer * 8 + 2;
        if (vy >= 200 || vy < -(int)PAL_RLEGetHeight(lpFrame)) {
            //
            // outside the screen; skip it
            //
            continue;
        }

        //
        // Add it into the array
        //
        addSpriteToDraw(i, lpFrame, x, y, lpEvtObj->sLayer * 8 + 2);

        //
        // Calculate covering map tiles
        //
        calcCoverTiles(&_rgSpriteToDraw[_nSpriteToDraw - 1]);
    }

    //
    // All sprites are now in our array; sort them by their vertical positions.
    //
    for (x = 0; x < _nSpriteToDraw - 1; x++) {
        SPRITE_TO_DRAW tmp;
        BOOL fSwap = FALSE;

        for (y = 0; y < _nSpriteToDraw - 1 - x; y++) {
            if (PAL_Y(_rgSpriteToDraw[y].pos) > PAL_Y(_rgSpriteToDraw[y + 1].pos)) {
                fSwap = TRUE;

                tmp = _rgSpriteToDraw[y];
                _rgSpriteToDraw[y] = _rgSpriteToDraw[y + 1];
                _rgSpriteToDraw[y + 1] = tmp;
            }
        }

        if (!fSwap) {
            break;
        }
    }

    //
    // Draw all the sprites to the screen.
    //
    for (i = 0; i < _nSpriteToDraw; i++) {
        SPRITE_TO_DRAW* p = &_rgSpriteToDraw[i];

        x = PAL_X(p->pos);
        y = PAL_Y(p->pos) - PAL_RLEGetHeight(p->lpSpriteFrame) - p->iLayer;

        bool spriteHovered = false;
        if (gpHighlightHoverSprites) {
            if (spriteHitTest(p->lpSpriteFrame, _renderer->getScreen(), PAL_XY(x, y), gpHoverPosX, gpHoverPosY) ||
             (gpHoveredObject != -1 && gpHoveredObject == p->i + 1)) {
                // rect hit
                spriteHovered = true;
            }
        }

        if (spriteHovered) {
            gpHighlightWidth = 1;
            gpHighlightPaletteIndex = 255;
            gpHoveredObject = p->i + 1;
        }
        PAL_RLEBlitToSurface(p->lpSpriteFrame, _renderer->getScreen(), PAL_XY(x, y));
        if (spriteHovered) {
            gpHighlightWidth = 0;
        }
    }
}

void PalScene::centerObject(WORD wEventObjectID, engine::LPEVENTOBJECT pObject)
{
    LPCBITMAPRLE lpFrame = nullptr;
    auto lpSprite = _resources->getEventObjectSprite(wEventObjectID);
    if (lpSprite) {
        auto iFrame = pObject->wCurrentFrameNum;
        if (pObject->nSpriteFrames == 3) {
            //
            // walking character
            //
            if (iFrame == 2) {
                iFrame = 0;
            }

            if (iFrame == 3) {
                iFrame = 2;
            }
        }

        lpFrame = _resources->spriteGetFrame(lpSprite,
            pObject->wDirection * pObject->nSpriteFrames + iFrame);
    }
    WORD x = pObject->x;
    WORD y = pObject->y;
    if (lpFrame) {
        // adjust xy for frame
        x -= PAL_RLEGetWidth(lpFrame) / 2;
        y += pObject->sLayer * 8 + 9;
        y -= PAL_RLEGetHeight(lpFrame) - pObject->sLayer * 8 + 2;
    }

    x -= SCENE_WIDTH / 2;
    y -= SCENE_HEIGHT / 2;

    _globals->getViewport() = PAL_XY(x, y);
}

void PalScene::addSpriteToDraw(int i, LPCBITMAPRLE lpSpriteFrame, int x, int y, int iLayer)
{
    assert(_nSpriteToDraw < MAX_SPRITE_TO_DRAW);
    _rgSpriteToDraw[_nSpriteToDraw].i = i;
    _rgSpriteToDraw[_nSpriteToDraw].lpSpriteFrame = lpSpriteFrame;
    _rgSpriteToDraw[_nSpriteToDraw].pos = PAL_XY(x, y);
    _rgSpriteToDraw[_nSpriteToDraw].iLayer = iLayer;

    _nSpriteToDraw++;
}

void PalScene::calcCoverTiles(SPRITE_TO_DRAW* lpSpriteToDraw)
{
    int x, y, i, l, iTileHeight;
    LPCBITMAPRLE lpTile;

    const int sx = PAL_X(gpGlobals->viewport) + PAL_X(lpSpriteToDraw->pos) - lpSpriteToDraw->iLayer / 2;
    const int sy = PAL_Y(gpGlobals->viewport) + PAL_Y(lpSpriteToDraw->pos) - lpSpriteToDraw->iLayer;
    const int sh = ((sx % 32) ? 1 : 0);

    const int width = PAL_RLEGetWidth(lpSpriteToDraw->lpSpriteFrame);
    const int height = PAL_RLEGetHeight(lpSpriteToDraw->lpSpriteFrame);

    int dx = 0;
    int dy = 0;
    int dh = 0;

    //
    // Loop through all the tiles in the area of the sprite.
    //
    for (y = (sy - height - 15) / 16; y <= sy / 16; y++) {
        for (x = (sx - width / 2) / 32; x <= (sx + width / 2) / 32; x++) {
            for (i = ((x == (sx - width / 2) / 32) ? 0 : 3); i < 5; i++) {
                //
                // Scan tiles in the following form (* = to scan):
                //
                // . . . * * * . . .
                //  . . . * * . . . .
                //
                switch (i) {
                case 0:
                    dx = x;
                    dy = y;
                    dh = sh;
                    break;

                case 1:
                    dx = x - 1;
                    break;

                case 2:
                    dx = (sh ? x : (x - 1));
                    dy = (sh ? (y + 1) : y);
                    dh = 1 - sh;
                    break;

                case 3:
                    dx = x + 1;
                    dy = y;
                    dh = sh;
                    break;

                case 4:
                    dx = (sh ? (x + 1) : x);
                    dy = (sh ? (y + 1) : y);
                    dh = 1 - sh;
                    break;
                }

                for (l = 0; l < 2; l++) {
                    lpTile = PAL_MapGetTileBitmap(dx, dy, dh, l, PAL_GetCurrentMap());
                    iTileHeight = (signed char)PAL_MapGetTileHeight(dx, dy, dh, l, PAL_GetCurrentMap());

                    //
                    // Check if this tile may cover the sprites
                    //
                    if (lpTile != NULL && iTileHeight > 0 && (dy + iTileHeight) * 16 + dh * 8 >= sy) {
                        //
                        // This tile may cover the sprite
                        //
                        addSpriteToDraw(-1, lpTile,
                            dx * 32 + dh * 16 - 16 - PAL_X(gpGlobals->viewport),
                            dy * 16 + dh * 8 + 7 + l + iTileHeight * 8 - PAL_Y(gpGlobals->viewport),
                            iTileHeight * 8 + l);
                    }
                }
            }
        }
    }
}

}
