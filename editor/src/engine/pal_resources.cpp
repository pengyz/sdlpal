#include "pal_resources.h"
#include "audio.h"
#include "pal_global.h"
#include "util.h"

namespace engine {

LPPALMAP PalResources::getCurrentMap()
{
    if (_resources == nullptr) {
        return nullptr;
    }

    return _resources->lpMap;
    // return PAL_GetCurrentMap();
}

void PalResources::init()
{
    _resources = (LPRESOURCES)calloc(1, sizeof(RESOURCES));
}

void PalResources::deinit()
{
    if (_resources != NULL) {
        //
        // Free all loaded sprites
        //
        freePlayerSprites();
        freeEventObjectSprites();

        //
        // Free map
        //
        PAL_FreeMap(_resources->lpMap);

        //
        // Delete the instance
        //
        free(_resources);
    }

    _resources = NULL;
}

void PalResources::freeResources()
{
    if (_resources != nullptr) {
        //
        // Free all loaded sprites
        //
        freePlayerSprites();
        freeEventObjectSprites();

        //
        // Free map
        //
        PAL_FreeMap(_resources->lpMap);

        //
        // Delete the instance
        //
        free(_resources);
    }

    _resources = nullptr;
}

void PalResources::freePlayerSprites()
{
    int i;

    for (i = 0; i < MAX_PLAYABLE_PLAYER_ROLES; i++) {
        free(_resources->rglpPlayerSprite[i]);
        _resources->rglpPlayerSprite[i] = nullptr;
    }
}

void PalResources::freeEventObjectSprites()
{
    int i;

    if (_resources->lppEventObjectSprites != nullptr) {
        for (i = 0; i < _resources->nEventObject; i++) {
            free(_resources->lppEventObjectSprites[i]);
        }

        free(_resources->lppEventObjectSprites);

        _resources->lppEventObjectSprites = nullptr;
        _resources->nEventObject = 0;
    }
}

void PalResources::loadResources()
{
    int i, index, l, n;
    WORD wPlayerID, wSpriteNum;

    if (_resources == NULL || _resources->bLoadFlags == 0) {
        return;
    }

    //
    // Load global data
    //
    if (_resources->bLoadFlags & kLoadGlobalData) {
        _globals->initGameData(_globals->getCurrentSaveSlot());
        AUDIO_PlayMusic(_globals->getNumMusic(), TRUE, 1);
    }

    //
    // Load scene
    //
    if (_resources->bLoadFlags & kLoadScene) {
        FILE *fpMAP, *fpGOP;

        fpMAP = UTIL_OpenRequiredFile("map.mkf");
        fpGOP = UTIL_OpenRequiredFile("gop.mkf");

        if (_globals->getEnteringScene()) {
            _globals->getScreenWave() = 0;
            _globals->getWaveProgression() = 0;
        }

        //
        // Free previous loaded scene (sprites and map)
        //
        freeEventObjectSprites();
        PAL_FreeMap(_resources->lpMap);

        //
        // Load map
        //
        i = _globals->getNumScene() - 1;
        _resources->lpMap = PAL_LoadMap(_globals->getGameData().rgScene[i].wMapNum,
            fpMAP, fpGOP);

        if (_resources->lpMap == NULL) {
            fclose(fpMAP);
            fclose(fpGOP);

            TerminateOnError("PAL_LoadResources(): Fail to load map #%d (scene #%d) !",
                _globals->getGameData().rgScene[i].wMapNum, _globals->getNumScene());
        }

        //
        // Load sprites
        //
        index = _globals->getGameData().rgScene[i].wEventObjectIndex;
        _resources->nEventObject = _globals->getGameData().rgScene[i + 1].wEventObjectIndex;
        _resources->nEventObject -= index;

        if (_resources->nEventObject > 0) {
            _resources->lppEventObjectSprites = (LPSPRITE*)UTIL_calloc(_resources->nEventObject, sizeof(LPSPRITE));
        }

        for (i = 0; i < _resources->nEventObject; i++, index++) {
            n = _globals->getGameData().lprgEventObject[index].wSpriteNum;
            if (n == 0) {
                //
                // this event object has no sprite
                //
                _resources->lppEventObjectSprites[i] = NULL;
                continue;
            }

            l = PAL_MKFGetDecompressedSize(n, _globals->getFiles().fpMGO);

            _resources->lppEventObjectSprites[i] = (LPSPRITE)UTIL_malloc(l);

            if (PAL_MKFDecompressChunk(_resources->lppEventObjectSprites[i], l,
                    n, _globals->getFiles().fpMGO)
                > 0) {
                _globals->getGameData().lprgEventObject[index].nSpriteFramesAuto = PAL_SpriteGetNumFrames(_resources->lppEventObjectSprites[i]);
            }
        }

        _globals->getPartyoffset() = PAL_XY(160, 112);

        fclose(fpGOP);
        fclose(fpMAP);
    }

    //
    // Load player sprites
    //
    if (_resources->bLoadFlags & kLoadPlayerSprite) {
        //
        // Free previous loaded player sprites
        //
        freePlayerSprites();

        for (i = 0; i <= (short)_globals->getMaxPartyMemberIndex(); i++) {
            wPlayerID = _globals->getrgParty()[i].wPlayerRole;
            assert(wPlayerID < MAX_PLAYER_ROLES);

            //
            // Load player sprite
            //
            wSpriteNum = _globals->getGameData().PlayerRoles.rgwSpriteNum[wPlayerID];

            l = PAL_MKFGetDecompressedSize(wSpriteNum, _globals->getFiles().fpMGO);

            _resources->rglpPlayerSprite[i] = (LPSPRITE)UTIL_malloc(l);

            PAL_MKFDecompressChunk(_resources->rglpPlayerSprite[i], l, wSpriteNum,
                _globals->getFiles().fpMGO);
        }

        for (i = 1; i <= _globals->getFollower(); i++) {
            //
            // Load the follower sprite
            //
            wSpriteNum = _globals->getrgParty()[(short)_globals->getMaxPartyMemberIndex() + i].wPlayerRole;

            l = PAL_MKFGetDecompressedSize(wSpriteNum, _globals->getFiles().fpMGO);

            _resources->rglpPlayerSprite[(short)_globals->getMaxPartyMemberIndex() + i] = (LPSPRITE)UTIL_malloc(l);

            PAL_MKFDecompressChunk(_resources->rglpPlayerSprite[(short)_globals->getMaxPartyMemberIndex() + i], l, wSpriteNum,
                _globals->getFiles().fpMGO);
        }
    }

    //
    // Clear all of the load flags
    //
    _resources->bLoadFlags = 0;
}

void PalResources::setLoadFlags(BYTE bFlags)
{
    if (_resources == NULL) {
        return;
    }

    _resources->bLoadFlags |= bFlags;
}

LPSPRITE PalResources::getPlayerSprite(BYTE bPlayerIndex)
{
    if (_resources == NULL || bPlayerIndex > MAX_PLAYABLE_PLAYER_ROLES - 1) {
        return NULL;
    }

    return _resources->rglpPlayerSprite[bPlayerIndex];
}

LPCBITMAPRLE PalResources::spriteGetFrame(LPCSPRITE lpSprite, INT iFrameNum)
{
    int imagecount, offset;

    if (lpSprite == NULL) {
        return NULL;
    }

    //
    // Hack for broken sprites like the Bloody-Mouth Bug
    //
    //   imagecount = (lpSprite[0] | (lpSprite[1] << 8)) - 1;
    imagecount = (lpSprite[0] | (lpSprite[1] << 8));

    if (iFrameNum < 0 || iFrameNum >= imagecount) {
        //
        // The frame does not exist
        //
        return NULL;
    }

    //
    // Get the offset of the frame
    //
    iFrameNum <<= 1;
    offset = ((lpSprite[iFrameNum] | (lpSprite[iFrameNum + 1] << 8)) << 1);
    if (offset == 0x18444)
        offset = (WORD)offset;
    return &lpSprite[offset];
}

LPSPRITE PalResources::getEventObjectSprite(WORD wEventObjectID)
{
    wEventObjectID -= _globals->getGameData().rgScene[_globals->getNumScene() - 1].wEventObjectIndex;
    wEventObjectID--;

    if (_resources == NULL || wEventObjectID >= _resources->nEventObject) {
        return NULL;
    }

    return _resources->lppEventObjectSprites[wEventObjectID];
}

}