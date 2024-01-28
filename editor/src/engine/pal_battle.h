#pragma once
#include "battle.h"

namespace engine {
class PalVideo;
class PalFight;
class PalGlobals;
class PalRenderer;
class PalScene;

class PalBattle {
public:
    PalBattle(PalGlobals* globals, PalRenderer* renderer, PalVideo* video, PalFight* fight, PalScene* scene)
        : _globals(globals)
        , _renderer(renderer)
        , _video(video)
        , _fight(fight)
        , _scene(scene)
    {
    }

    int startBattle(WORD wEnemyTeam, bool fIsBoss);

    void loadBattleSprites();
    void battleMakeScene();

private:
    void loadBattleBackground();
    void freeBattleSprites();

private:
    BATTLE _battle;
    PalGlobals* _globals = nullptr;
    PalRenderer* _renderer = nullptr;
    PalFight* _fight = nullptr;
    PalVideo* _video = nullptr;
    PalScene* _scene = nullptr;
    WORD _rgPlayerPos[3][3][2] = {
        { { 240, 170 } }, // one player
        { { 200, 176 }, { 256, 152 } }, // two players
        { { 180, 180 }, { 234, 170 }, { 270, 146 } } // three players
    };
};
}