#include "pal_battle.h"
#include "common.h"
#include "pal_fight.h"
#include "pal_global.h"
#include "pal_renderer.h"
#include "pal_scene.h"
#include "pal_video.h"
#include "palcommon.h"
#include "util.h"

namespace engine {

void PalBattle::loadBattleBackground()
{
    PAL_LARGE BYTE buf[320 * 200];

    //
    // Create the surface
    //
    _battle.lpBackground = _video->createCompatibleSizedSurface(_renderer->getScreen(), nullptr);

    if (_battle.lpBackground == NULL) {
        assert(false && "PAL_LoadBattleBackground(): failed to create surface!");
    }

    //
    // Load the picture
    //
    PAL_MKFDecompressChunk(buf, 320 * 200, _globals->getNumBattleField(), _globals->getFiles().fpFBP);

    //
    // Draw the picture to the surface.
    //
    PAL_FBPBlitToSurface(buf, _battle.lpBackground);
}

int PalBattle::startBattle(WORD wEnemyTeam, bool fIsBoss)
{
    int i;
    WORD w, wPrevWaveLevel;
    SHORT sPrevWaveProgression;

    //
    // Set the screen waving effects
    //
    wPrevWaveLevel = _globals->getScreenWave();
    sPrevWaveProgression = _globals->getWaveProgression();

    _globals->getWaveProgression() = 0;
    _globals->getScreenWave() = _globals->getGameData().lprgBattleField[_globals->getNumBattleField()].wScreenWave;

    //
    // Make sure everyone in the party is alive, also clear all hidden
    // EXP count records
    //
    for (i = 0; i <= _globals->getMaxPartyMemberIndex(); i++) {
        w = _globals->getrgParty()[i].wPlayerRole;

        if (_globals->getGameData().PlayerRoles.rgwHP[w] == 0) {
            _globals->getGameData().PlayerRoles.rgwHP[w] = 1;
            _globals->getrgPlayerStatus()[w][kStatusPuppet] = 0;
        }

        _globals->getExp().rgHealthExp[w].wCount = 0;
        _globals->getExp().rgMagicExp[w].wCount = 0;
        _globals->getExp().rgAttackExp[w].wCount = 0;
        _globals->getExp().rgMagicPowerExp[w].wCount = 0;
        _globals->getExp().rgDefenseExp[w].wCount = 0;
        _globals->getExp().rgDexterityExp[w].wCount = 0;
        _globals->getExp().rgFleeExp[w].wCount = 0;
    }

    //
    // Clear all item-using records
    //
    for (i = 0; i < MAX_INVENTORY; i++) {
        _globals->getrgInventory()[i].nAmountInUse = 0;
    }

    //
    // Store all enemies
    //
    for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++) {
        memset(&(_battle.rgEnemy[i]), 0, sizeof(BATTLEENEMY));
        w = _globals->getGameData().lprgEnemyTeam[wEnemyTeam].rgwEnemy[i];

        if (w == 0xFFFF) {
            break;
        }

        if (w != 0) {
            LPENEMY enemy = &_globals->getGameData().lprgEnemy[_globals->getGameData().rgObject[w].enemy.wEnemyID];
            memcpy(&_battle.rgEnemy[i].e, enemy, sizeof(*enemy));
            _battle.rgEnemy[i].wObjectID = w;
            _battle.rgEnemy[i].state = kFighterWait;
            _battle.rgEnemy[i].wScriptOnTurnStart = _globals->getGameData().rgObject[w].enemy.wScriptOnTurnStart;
            _battle.rgEnemy[i].wScriptOnBattleEnd = _globals->getGameData().rgObject[w].enemy.wScriptOnBattleEnd;
            _battle.rgEnemy[i].wScriptOnReady = _globals->getGameData().rgObject[w].enemy.wScriptOnReady;
            _battle.rgEnemy[i].iColorShift = 0;
        }
    }

    _battle.wMaxEnemyIndex = i - 1;

    //
    // Store all players
    //
    for (i = 0; i <= _globals->getMaxPartyMemberIndex(); i++) {
        _battle.rgPlayer[i].flTimeMeter = 15.0f;
        _battle.rgPlayer[i].wHidingTime = 0;
        _battle.rgPlayer[i].state = kFighterWait;
        _battle.rgPlayer[i].fDefending = FALSE;
        _battle.rgPlayer[i].wCurrentFrame = 0;
        _battle.rgPlayer[i].iColorShift = FALSE;
    }

    //
    // Load sprites and background
    //
    loadBattleSprites();
    loadBattleBackground();
    //
    // Create the surface for scene buffer
    //
    _battle.lpSceneBuf = _video->createCompatibleSizedSurface(_renderer->getScreen(), nullptr);

    if (_battle.lpSceneBuf == nullptr) {
        TerminateOnError("PAL_StartBattle(): creating surface for scene buffer failed!");
    }

    _globals->updateEquipments();

    _battle.iExpGained = 0;
    _battle.iCashGained = 0;

    _battle.fIsBoss = fIsBoss;
    _battle.fEnemyCleared = FALSE;
    _battle.fEnemyMoving = FALSE;
    _battle.iHidingTime = 0;
    _battle.wMovingPlayerIndex = 0;

    _battle.UI.szMsg[0] = '\0';
    _battle.UI.szNextMsg[0] = '\0';
    _battle.UI.dwMsgShowTime = 0;
    _battle.UI.state = kBattleUIWait;
    _battle.UI.fAutoAttack = FALSE;
    _battle.UI.iSelectedIndex = 0;
    _battle.UI.iPrevEnemyTarget = -1;

    memset(_battle.UI.rgShowNum, 0, sizeof(_battle.UI.rgShowNum));

    _battle.lpSummonSprite = NULL;
    _battle.sBackgroundColorShift = 0;

    _globals->getInBattle() = TRUE;
    _battle.BattleResult = kBattleResultPreBattle;

    _fight->battleUpdateFighters();

    //
    // Load the battle effect sprite.
    //
    i = PAL_MKFGetChunkSize(10, _globals->getFiles().fpDATA);
    _battle.lpEffectSprite = static_cast<LPBYTE>(UTIL_malloc(i));

    PAL_MKFReadChunk(_battle.lpEffectSprite, i, 10, _globals->getFiles().fpDATA);

#ifdef PAL_CLASSIC
    _battle.Phase = kBattlePhaseSelectAction;
    _battle.fRepeat = FALSE;
    _battle.fForce = FALSE;
    _battle.fFlee = FALSE;
    _battle.fPrevAutoAtk = FALSE;
    _battle.fThisTurnCoop = FALSE;
#endif

    //
    // Run the main battle routine.
    //
#if 0
        i = PAL_BattleMain();

        if (i == kBattleResultWon) {
            //
            // Player won the battle. Add the Experience points.
            //
            PAL_BattleWon();
        }
#endif
    // clear logic
#if 0
    //
    // Clear all item-using records
    //
    for (w = 0; w < MAX_INVENTORY; w++) {
        _globals->getrgInventory()[w].nAmountInUse = 0;
    }

    //
    // Clear all player status, poisons and temporary effects
    //
    _globals->clearAllPlayerStatus();
    for (w = 0; w < MAX_PLAYER_ROLES; w++) {
        PAL_CurePoisonByLevel(w, 3);
        PAL_RemoveEquipmentEffect(w, kBodyPartExtra);
    }

    //
    // Free all the battle sprites
    //
    freeBattleSprites();
    free(_battle.lpEffectSprite);

    //
    // Free the surfaces for the background picture and scene buffer
    //
    VIDEO_FreeSurface(_battle.lpBackground);
    VIDEO_FreeSurface(_battle.lpSceneBuf);

    _battle.lpBackground = NULL;
    _battle.lpSceneBuf = NULL;

    _globals->fInBattle = FALSE;

    AUDIO_PlayMusic(_globals->getNumMusic(), TRUE, 1);

    //
    // Restore the screen waving effects
    //
    _globals->getWaveProgression() = sPrevWaveProgression;
    _globals->getScreenWave() = wPrevWaveLevel;
#endif

    return i;
}

void PalBattle::loadBattleSprites()
{
    int i, l, x, y, s;
    FILE* fp;

    freeBattleSprites();

    fp = UTIL_OpenRequiredFile("abc.mkf");

    //
    // Load battle sprites for players
    //
    for (i = 0; i <= _globals->getMaxPartyMemberIndex(); i++) {
        s = _globals->getPlayerBattleSprite(_globals->getrgParty()[i].wPlayerRole);

        l = PAL_MKFGetDecompressedSize(s, _globals->getFiles().fpF);

        if (l <= 0) {
            continue;
        }

        _battle.rgPlayer[i].lpSprite = static_cast<LPSPRITE>(UTIL_calloc(l, 1));

        PAL_MKFDecompressChunk(_battle.rgPlayer[i].lpSprite, l,
            s, _globals->getFiles().fpF);

        //
        // Set the default position for this player
        //
        x = _rgPlayerPos[_globals->getMaxPartyMemberIndex()][i][0];
        y = _rgPlayerPos[_globals->getMaxPartyMemberIndex()][i][1];

        _battle.rgPlayer[i].posOriginal = PAL_XY(x, y);
        _battle.rgPlayer[i].pos = PAL_XY(x, y);
    }

    //
    // Load battle sprites for enemies
    //
    for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++) {
        if (_battle.rgEnemy[i].wObjectID == 0) {
            continue;
        }

        l = PAL_MKFGetDecompressedSize(
            _globals->getGameData().rgObject[_battle.rgEnemy[i].wObjectID].enemy.wEnemyID, fp);

        if (l <= 0) {
            continue;
        }

        _battle.rgEnemy[i].lpSprite = static_cast<LPSPRITE>(UTIL_calloc(l, 1));

        PAL_MKFDecompressChunk(_battle.rgEnemy[i].lpSprite, l,
            _globals->getGameData().rgObject[_battle.rgEnemy[i].wObjectID].enemy.wEnemyID, fp);

        //
        // Set the default position for this enemy
        //
        x = _globals->getGameData().EnemyPos.pos[i][_battle.wMaxEnemyIndex].x;
        y = _globals->getGameData().EnemyPos.pos[i][_battle.wMaxEnemyIndex].y;

        y += _battle.rgEnemy[i].e.wYPosOffset;

        _battle.rgEnemy[i].posOriginal = PAL_XY(x, y);
        _battle.rgEnemy[i].pos = PAL_XY(x, y);
    }

    fclose(fp);
}

void PalBattle::freeBattleSprites()
{
    int i;

    //
    // Free all the loaded sprites
    //
    for (i = 0; i <= _globals->getMaxPartyMemberIndex(); i++) {
        if (_battle.rgPlayer[i].lpSprite != NULL) {
            free(_battle.rgPlayer[i].lpSprite);
        }
        _battle.rgPlayer[i].lpSprite = NULL;
    }

    for (i = 0; i <= _battle.wMaxEnemyIndex; i++) {
        if (_battle.rgEnemy[i].lpSprite != NULL) {
            free(_battle.rgEnemy[i].lpSprite);
        }
        _battle.rgEnemy[i].lpSprite = NULL;
    }

    if (_battle.lpSummonSprite != NULL) {
        free(_battle.lpSummonSprite);
    }
    _battle.lpSummonSprite = NULL;
}

void PalBattle::battleMakeScene()
{
    int i, j;
    PAL_POS pos;
    LPBYTE pSrc, pDst;
    BYTE b;
    INT enemyDrawSeq[MAX_ENEMIES_IN_TEAM];

    //
    // Draw the background
    //
    pSrc = static_cast<LPBYTE>(_battle.lpBackground->pixels);
    pDst = static_cast<LPBYTE>(_battle.lpSceneBuf->pixels);

    for (i = 0; i < _battle.lpSceneBuf->pitch * _battle.lpSceneBuf->h; i++) {
        b = (*pSrc & 0x0F);
        b += _battle.sBackgroundColorShift;

        if (b & 0x80) {
            b = 0;
        } else if (b & 0x70) {
            b = 0x0F;
        }

        *pDst = (b | (*pSrc & 0xF0));

        ++pSrc;
        ++pDst;
    }

    _scene->applyWave(_battle.lpSceneBuf);

    memset(&enemyDrawSeq, -1, sizeof(enemyDrawSeq));
    // sort by y
    for (i = 0; i <= _battle.wMaxEnemyIndex; i++)
        enemyDrawSeq[i] = i;
    for (i = 0; i < _battle.wMaxEnemyIndex; i++)
        for (j = i + 1; j < _battle.wMaxEnemyIndex; j++)
            if (PAL_Y(_battle.rgEnemy[i].pos) < PAL_Y(_battle.rgEnemy[j].pos)) {
                INT tmp = enemyDrawSeq[i];
                enemyDrawSeq[i] = enemyDrawSeq[j];
                enemyDrawSeq[j] = tmp;
            }

    //
    // Draw the enemies
    //
    for (j = _battle.wMaxEnemyIndex; j >= 0; j--) {
        i = enemyDrawSeq[j];
        pos = _battle.rgEnemy[i].pos;

        if (_battle.rgEnemy[i].rgwStatus[kStatusConfused] > 0 && _battle.rgEnemy[i].rgwStatus[kStatusSleep] == 0 && _battle.rgEnemy[i].rgwStatus[kStatusParalyzed] == 0) {
            //
            // Enemy is confused
            //
            pos = PAL_XY(PAL_X(pos) + RandomLong(-1, 1), PAL_Y(pos));
        }

        pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(PAL_SpriteGetFrame(_battle.rgEnemy[i].lpSprite, _battle.rgEnemy[i].wCurrentFrame)) / 2,
            PAL_Y(pos) - PAL_RLEGetHeight(PAL_SpriteGetFrame(_battle.rgEnemy[i].lpSprite, _battle.rgEnemy[i].wCurrentFrame)));

        if (_battle.rgEnemy[i].wObjectID != 0) {
            if (_battle.rgEnemy[i].iColorShift) {
                PAL_RLEBlitWithColorShift(PAL_SpriteGetFrame(_battle.rgEnemy[i].lpSprite, _battle.rgEnemy[i].wCurrentFrame),
                    _battle.lpSceneBuf, pos, _battle.rgEnemy[i].iColorShift);
            } else {
                PAL_RLEBlitToSurface(PAL_SpriteGetFrame(_battle.rgEnemy[i].lpSprite, _battle.rgEnemy[i].wCurrentFrame),
                    _battle.lpSceneBuf, pos);
            }
        }
    }

    if (_battle.lpSummonSprite != NULL) {
        //
        // Draw the summoned god
        //
        pos = PAL_XY(PAL_X(_battle.posSummon) - PAL_RLEGetWidth(PAL_SpriteGetFrame(_battle.lpSummonSprite, _battle.iSummonFrame)) / 2,
            PAL_Y(_battle.posSummon) - PAL_RLEGetHeight(PAL_SpriteGetFrame(_battle.lpSummonSprite, _battle.iSummonFrame)));

        PAL_RLEBlitToSurface(PAL_SpriteGetFrame(_battle.lpSummonSprite, _battle.iSummonFrame),
            _battle.lpSceneBuf, pos);
    } else {
        //
        // Draw the players
        //
        for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--) {
            pos = _battle.rgPlayer[i].pos;

            if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusConfused] != 0 && gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusSleep] == 0 && gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusParalyzed] == 0 && gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0) {
                //
                // Player is confused
                //
                continue;
            }

            pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame)) / 2,
                PAL_Y(pos) - PAL_RLEGetHeight(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame)));

            if (_battle.rgPlayer[i].iColorShift != 0) {
                PAL_RLEBlitWithColorShift(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame),
                    _battle.lpSceneBuf, pos, _battle.rgPlayer[i].iColorShift);
            } else if (_battle.iHidingTime == 0) {
                PAL_RLEBlitToSurface(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame),
                    _battle.lpSceneBuf, pos);
            }
        }

        //
        // Confused players should be drawn on top of normal players
        //
        for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--) {
            if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusConfused] != 0 && gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusSleep] == 0 && gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusParalyzed] == 0 && gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0) {
                //
                // Player is confused
                //
                int xd = PAL_X(_battle.rgPlayer[i].pos), yd = PAL_Y(_battle.rgPlayer[i].pos);
                if (!_fight->isPlayerDying(gpGlobals->rgParty[i].wPlayerRole))
                    yd += RandomLong(-1, 1);
                pos = PAL_XY(xd, yd);
                pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame)) / 2,
                    PAL_Y(pos) - PAL_RLEGetHeight(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame)));

                if (_battle.rgPlayer[i].iColorShift != 0) {
                    PAL_RLEBlitWithColorShift(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame),
                        _battle.lpSceneBuf, pos, _battle.rgPlayer[i].iColorShift);
                } else if (_battle.iHidingTime == 0) {
                    PAL_RLEBlitToSurface(PAL_SpriteGetFrame(_battle.rgPlayer[i].lpSprite, _battle.rgPlayer[i].wCurrentFrame),
                        _battle.lpSceneBuf, pos);
                }
            }
        }
    }
}

}