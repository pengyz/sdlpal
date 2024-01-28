#pragma once
#include "palcommon.h"

namespace engine {
class PalGlobals;
class PalBattle;
class PalFight {

public:
    PalFight(PalGlobals* globals)
        : _globals(globals)
    {
    }
    void battleUpdateFighters();
    bool isPlayerDying(WORD wPlayerRole);

private:
    PalGlobals* _globals = nullptr;
    PalBattle* _battle = nullptr;
};
}