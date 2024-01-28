#include "pal_fight.h"
#include "pal_battle.h"
#include "pal_global.h"

namespace engine {

void PalFight::battleUpdateFighters()
{
    int i;
    WORD wPlayerRole;

    //
    // Update the gesture for all players
    //
    for (i = 0; i <= _globals->getMaxPartyMemberIndex(); i++) {
        wPlayerRole = _globals->getrgParty()[i].wPlayerRole;

        if (!g_Battle.rgPlayer[i].fDefending)
            g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
        g_Battle.rgPlayer[i].iColorShift = 0;

        if (_globals->getGameData().PlayerRoles.rgwHP[wPlayerRole] == 0) {
            if (_globals->getrgPlayerStatus()[wPlayerRole][kStatusPuppet] == 0) {
                g_Battle.rgPlayer[i].wCurrentFrame = 2; // dead
            } else {
                g_Battle.rgPlayer[i].wCurrentFrame = 0; // puppet
            }
        } else {
            if (_globals->getrgPlayerStatus()[wPlayerRole][kStatusSleep] != 0 || isPlayerDying(wPlayerRole)) {
                g_Battle.rgPlayer[i].wCurrentFrame = 1;
            }
#ifndef PAL_CLASSIC
            else if (g_Battle.rgPlayer[i].state == kFighterAct && g_Battle.rgPlayer[i].action.ActionType == kBattleActionMagic && !g_Battle.fEnemyCleared) {
                //
                // Player is using a magic
                //
                g_Battle.rgPlayer[i].wCurrentFrame = 5;
            }
#endif
            else if (g_Battle.rgPlayer[i].fDefending && !g_Battle.fEnemyCleared) {
                g_Battle.rgPlayer[i].wCurrentFrame = 3;
            } else {
                g_Battle.rgPlayer[i].wCurrentFrame = 0;
            }
        }
    }

    //
    // Update the gesture for all enemies
    //
    for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++) {
        if (g_Battle.rgEnemy[i].wObjectID == 0) {
            continue;
        }

        g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
        g_Battle.rgEnemy[i].iColorShift = 0;

        if (g_Battle.rgEnemy[i].rgwStatus[kStatusSleep] > 0 || g_Battle.rgEnemy[i].rgwStatus[kStatusParalyzed] > 0) {
            g_Battle.rgEnemy[i].wCurrentFrame = 0;
            continue;
        }

        if (--g_Battle.rgEnemy[i].e.wIdleAnimSpeed == 0) {
            g_Battle.rgEnemy[i].wCurrentFrame++;
            g_Battle.rgEnemy[i].e.wIdleAnimSpeed = _globals->getGameData().lprgEnemy[_globals->getGameData().rgObject[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
        }

        if (g_Battle.rgEnemy[i].wCurrentFrame >= g_Battle.rgEnemy[i].e.wIdleFrames) {
            g_Battle.rgEnemy[i].wCurrentFrame = 0;
        }
    }
}

bool PalFight::isPlayerDying(WORD wPlayerRole)
{
    return _globals->getGameData().PlayerRoles.rgwHP[wPlayerRole] < min(100, _globals->getGameData().PlayerRoles.rgwMaxHP[wPlayerRole] / 5);
}

}