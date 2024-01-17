#include "pal_scene.h"
#include "pal_input.h"
#include "global.h"
#include "pal_resources.h"

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

        xSource = PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset);
        ySource = PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset);

        xTarget = xSource + xOffset;
        yTarget = ySource + yOffset;

        gpGlobals->wPartyDirection = pInputState->dir;

        //
        // Check for obstacles on the destination location
        //
        if (!checkObstacle(PAL_XY(xTarget, yTarget), TRUE, 0)) {
            //
            // Player will actually be moved. Store trail.
            //
            for (i = 3; i >= 0; i--) {
                gpGlobals->rgTrail[i + 1] = gpGlobals->rgTrail[i];
            }

            gpGlobals->rgTrail[0].wDirection = pInputState->dir;
            gpGlobals->rgTrail[0].x = xSource;
            gpGlobals->rgTrail[0].y = ySource;

            //
            // Move the viewport
            //
            gpGlobals->viewport = PAL_XY(PAL_X(gpGlobals->viewport) + xOffset, PAL_Y(gpGlobals->viewport) + yOffset);

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

        gpGlobals->rgParty[0].x = PAL_X(gpGlobals->partyoffset);
        gpGlobals->rgParty[0].y = PAL_Y(gpGlobals->partyoffset);

        if (gpGlobals->g.PlayerRoles.rgwWalkFrames[gpGlobals->rgParty[0].wPlayerRole] == 4) {
            gpGlobals->rgParty[0].wFrame = gpGlobals->wPartyDirection * 4 + _iThisStepFrame;
        } else {
            gpGlobals->rgParty[0].wFrame = gpGlobals->wPartyDirection * 3 + iStepFrameLeader;
        }

        //
        // Update the gestures and positions for other party members
        //
        for (i = 1; i <= (short)gpGlobals->wMaxPartyMemberIndex; i++) {
            gpGlobals->rgParty[i].x = gpGlobals->rgTrail[1].x - PAL_X(gpGlobals->viewport);
            gpGlobals->rgParty[i].y = gpGlobals->rgTrail[1].y - PAL_Y(gpGlobals->viewport);

            if (i == 2) {
                gpGlobals->rgParty[i].x += (gpGlobals->rgTrail[1].wDirection == kDirEast || gpGlobals->rgTrail[1].wDirection == kDirWest) ? -16 : 16;
                gpGlobals->rgParty[i].y += 8;
            } else {
                gpGlobals->rgParty[i].x += ((gpGlobals->rgTrail[1].wDirection == kDirWest || gpGlobals->rgTrail[1].wDirection == kDirSouth) ? 16 : -16);
                gpGlobals->rgParty[i].y += ((gpGlobals->rgTrail[1].wDirection == kDirWest || gpGlobals->rgTrail[1].wDirection == kDirNorth) ? 8 : -8);
            }

            //
            // Adjust the position if there is obstacle
            //
            if (checkObstacle(PAL_XY(gpGlobals->rgParty[i].x + PAL_X(gpGlobals->viewport),
                                  gpGlobals->rgParty[i].y + PAL_Y(gpGlobals->viewport)),
                    TRUE, 0)) {
                gpGlobals->rgParty[i].x = gpGlobals->rgTrail[1].x - PAL_X(gpGlobals->viewport);
                gpGlobals->rgParty[i].y = gpGlobals->rgTrail[1].y - PAL_Y(gpGlobals->viewport);
            }

            //
            // Update gesture for this party member
            //
            if (gpGlobals->g.PlayerRoles.rgwWalkFrames[gpGlobals->rgParty[i].wPlayerRole] == 4) {
                gpGlobals->rgParty[i].wFrame = gpGlobals->rgTrail[2].wDirection * 4 + _iThisStepFrame;
            } else {
                gpGlobals->rgParty[i].wFrame = gpGlobals->rgTrail[2].wDirection * 3 + iStepFrameLeader;
            }
        }

        for (i = 1; i <= gpGlobals->nFollower; i++) {
            //
            // Update the position and gesture for the follower
            //
            gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + i].x = gpGlobals->rgTrail[2 + i].x - PAL_X(gpGlobals->viewport);
            gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + i].y = gpGlobals->rgTrail[2 + i].y - PAL_Y(gpGlobals->viewport);
            gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + i].wFrame = gpGlobals->rgTrail[2 + i].wDirection * 3 + iStepFrameFollower;
        }
    } else {
        //
        // Player is not moved. Use the "standing" gesture instead of "walking" one.
        //
        i = gpGlobals->g.PlayerRoles.rgwWalkFrames[gpGlobals->rgParty[0].wPlayerRole];
        if (i == 0) {
            i = 3;
        }
        gpGlobals->rgParty[0].wFrame = gpGlobals->wPartyDirection * i;

        for (i = 1; i <= (short)gpGlobals->wMaxPartyMemberIndex; i++) {
            int f = gpGlobals->g.PlayerRoles.rgwWalkFrames[gpGlobals->rgParty[i].wPlayerRole];
            if (f == 0) {
                f = 3;
            }
            gpGlobals->rgParty[i].wFrame = gpGlobals->rgTrail[2].wDirection * f;
        }

        for (i = 1; i <= gpGlobals->nFollower; i++) {
            gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + i].wFrame = gpGlobals->rgTrail[2 + i].wDirection * 3;
        }

        _iThisStepFrame &= 2;
        _iThisStepFrame ^= 2;
    }
}

bool PalScene::checkObstacle(PAL_POS pos, bool fCheckEventObjects, WORD wSelfObject)
{
    int x, y, h, xr, yr;
    int blockX = PAL_X(gpGlobals->partyoffset) / 32, blockY = PAL_Y(gpGlobals->partyoffset) / 16;

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
        for (i = gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wEventObjectIndex;
             i < gpGlobals->g.rgScene[gpGlobals->wNumScene].wEventObjectIndex; i++) {
            LPEVENTOBJECT p = &(gpGlobals->g.lprgEventObject[i]);
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

}
