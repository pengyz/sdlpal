#include "pal_loader.h"

#include "audio.h"
#include "font.h"
#include "global.h"
#include "palcfg.h"
#include "res.h"
#include "text.h"
#include "util.h"
#include <setjmp.h>

extern "C" {

static jmp_buf g_exit_jmp_buf;
static int g_exit_code = 0;

VOID PAL_Shutdown(
    int exit_code)
/*++
  Purpose:

    Free everything needed by the game.

  Parameters:

    exit_code -  The exit code return to OS.

  Return value:

    None.

--*/
{
    PAL_FreeFont();
    PAL_FreeResources();
    PAL_FreeUI();
    PAL_FreeText();
    //
    // global needs be free in last
    // since subsystems may needs config content during destroy
    // which also cleared here
    //
    PAL_FreeGlobals();

    g_exit_code = exit_code;
#if !__EMSCRIPTEN__
    longjmp(g_exit_jmp_buf, 1);
#else
    SDL_Quit();
    UTIL_Platform_Quit();
    return;
#endif
}
}

bool PalLoader::load()
{
    // load config
    PAL_LoadConfig(TRUE);
    int e = PAL_InitGlobals();
    if (e != 0) {
        TerminateOnError("Could not initialize global data: %d.\n", e);
    }
    e = PAL_InitText();
    if (e != 0) {
        TerminateOnError("Could not initialize text subsystem: %d.\n", e);
    }
    e = PAL_InitFont(&gConfig);
    if (e != 0) {
        TerminateOnError("Could not load fonts: %d.\n", e);
    }

    initResources();

    // set load flags
    gpGlobals->bCurrentSaveSlot = (BYTE)1;
    setLoadFlags(kLoadGlobalData | kLoadScene | kLoadPlayerSprite);
    gpGlobals->fEnteringScene = TRUE;
    gpGlobals->fNeedToFadeIn = TRUE;
    gpGlobals->dwFrameNum = 0;

    // load resources
    loadResources();
    return true;
}

void PalLoader::initResources()
{
    gpResources = (LPRESOURCES)UTIL_calloc(1, sizeof(RESOURCES));
}

void PalLoader::setLoadFlags(BYTE bFlags)
{
    if (gpResources == NULL) {
        return;
    }

    gpResources->bLoadFlags |= bFlags;
}

void PalLoader::loadResources()
{
    int i, index, l, n;
    WORD wPlayerID, wSpriteNum;

    if (gpResources == NULL || gpResources->bLoadFlags == 0) {
        return;
    }

    //
    // Load global data
    //
    if (gpResources->bLoadFlags & kLoadGlobalData) {
        PAL_InitGameData(gpGlobals->bCurrentSaveSlot);
    }

    //
    // Load scene
    //
    if (gpResources->bLoadFlags & kLoadScene) {
        FILE *fpMAP, *fpGOP;

        fpMAP = UTIL_OpenRequiredFile("map.mkf");
        fpGOP = UTIL_OpenRequiredFile("gop.mkf");

        if (gpGlobals->fEnteringScene) {
            gpGlobals->wScreenWave = 0;
            gpGlobals->sWaveProgression = 0;
        }

        //
        // Free previous loaded scene (sprites and map)
        //
        freeEventObjectSprites();
        PAL_FreeMap(gpResources->lpMap);

        //
        // Load map
        //
        i = gpGlobals->wNumScene - 1;
        gpResources->lpMap = PAL_LoadMap(gpGlobals->g.rgScene[i].wMapNum,
            fpMAP, fpGOP);

        if (gpResources->lpMap == NULL) {
            fclose(fpMAP);
            fclose(fpGOP);

            TerminateOnError("PAL_LoadResources(): Fail to load map #%d (scene #%d) !",
                gpGlobals->g.rgScene[i].wMapNum, gpGlobals->wNumScene);
        }

        //
        // Load sprites
        //
        index = gpGlobals->g.rgScene[i].wEventObjectIndex;
        gpResources->nEventObject = gpGlobals->g.rgScene[i + 1].wEventObjectIndex;
        gpResources->nEventObject -= index;

        if (gpResources->nEventObject > 0) {
            gpResources->lppEventObjectSprites = (LPSPRITE*)UTIL_calloc(gpResources->nEventObject, sizeof(LPSPRITE));
        }

        for (i = 0; i < gpResources->nEventObject; i++, index++) {
            n = gpGlobals->g.lprgEventObject[index].wSpriteNum;
            if (n == 0) {
                //
                // this event object has no sprite
                //
                gpResources->lppEventObjectSprites[i] = NULL;
                continue;
            }

            l = PAL_MKFGetDecompressedSize(n, gpGlobals->f.fpMGO);

            gpResources->lppEventObjectSprites[i] = (LPSPRITE)UTIL_malloc(l);

            if (PAL_MKFDecompressChunk(gpResources->lppEventObjectSprites[i], l,
                    n, gpGlobals->f.fpMGO)
                > 0) {
                gpGlobals->g.lprgEventObject[index].nSpriteFramesAuto = PAL_SpriteGetNumFrames(gpResources->lppEventObjectSprites[i]);
            }
        }

        gpGlobals->partyoffset = PAL_XY(160, 112);

        fclose(fpGOP);
        fclose(fpMAP);
    }

    //
    // Load player sprites
    //
    if (gpResources->bLoadFlags & kLoadPlayerSprite) {
        //
        // Free previous loaded player sprites
        //
        freePlayerSprites();

        for (i = 0; i <= (short)gpGlobals->wMaxPartyMemberIndex; i++) {
            wPlayerID = gpGlobals->rgParty[i].wPlayerRole;
            assert(wPlayerID < MAX_PLAYER_ROLES);

            //
            // Load player sprite
            //
            wSpriteNum = gpGlobals->g.PlayerRoles.rgwSpriteNum[wPlayerID];

            l = PAL_MKFGetDecompressedSize(wSpriteNum, gpGlobals->f.fpMGO);

            gpResources->rglpPlayerSprite[i] = (LPSPRITE)UTIL_malloc(l);

            PAL_MKFDecompressChunk(gpResources->rglpPlayerSprite[i], l, wSpriteNum,
                gpGlobals->f.fpMGO);
        }

        for (i = 1; i <= gpGlobals->nFollower; i++) {
            //
            // Load the follower sprite
            //
            wSpriteNum = gpGlobals->rgParty[(short)gpGlobals->wMaxPartyMemberIndex + i].wPlayerRole;

            l = PAL_MKFGetDecompressedSize(wSpriteNum, gpGlobals->f.fpMGO);

            gpResources->rglpPlayerSprite[(short)gpGlobals->wMaxPartyMemberIndex + i] = (LPSPRITE)UTIL_malloc(l);

            PAL_MKFDecompressChunk(gpResources->rglpPlayerSprite[(short)gpGlobals->wMaxPartyMemberIndex + i], l, wSpriteNum,
                gpGlobals->f.fpMGO);
        }
    }

    //
    // Clear all of the load flags
    //
    gpResources->bLoadFlags = 0;
}

void PalLoader::freeEventObjectSprites()
{
    int i;

    if (gpResources->lppEventObjectSprites != NULL) {
        for (i = 0; i < gpResources->nEventObject; i++) {
            free(gpResources->lppEventObjectSprites[i]);
        }

        free(gpResources->lppEventObjectSprites);

        gpResources->lppEventObjectSprites = NULL;
        gpResources->nEventObject = 0;
    }
}

void PalLoader::freePlayerSprites()
{
    int i;

    for (i = 0; i < MAX_PLAYABLE_PLAYER_ROLES; i++) {
        free(gpResources->rglpPlayerSprite[i]);
        gpResources->rglpPlayerSprite[i] = NULL;
    }
}