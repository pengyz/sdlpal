#include "pal_scene.h"
#include "global.h"
#include "pal_input.h"
#include "pal_resources.h"
#include "res.h"

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
        if (!checkObstacle(PAL_XY(xTarget, yTarget), TRUE, 0)) {
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
            updatePartyGestures(true);

            return; // don't go further
        }
    }

    updatePartyGestures(false);
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
        addSpriteToDraw(lpBitmap,
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

        if (x >= 320 || x < -(int)PAL_RLEGetWidth(lpFrame)) {
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
        addSpriteToDraw(lpFrame, x, y, lpEvtObj->sLayer * 8 + 2);

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

        PAL_RLEBlitToSurface(p->lpSpriteFrame, _renderer->getScreen(), PAL_XY(x, y));
    }
}

void PalScene::addSpriteToDraw(LPCBITMAPRLE lpSpriteFrame, int x, int y, int iLayer)
{
    assert(_nSpriteToDraw < MAX_SPRITE_TO_DRAW);

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
                        addSpriteToDraw(lpTile,
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
