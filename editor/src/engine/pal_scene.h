#pragma once

#include "pal_resources.h"
#include "palcommon.h"

namespace engine {
class PalInput;
struct PALINPUTSTATE;
class PalScene {
public:
    PalScene(PalResources* resources)
        : _resources(resources)
    {
    }

    void updateParty(PALINPUTSTATE* inputState);
    void updatePartyGestures(bool fWalking);
    bool checkObstacle(PAL_POS pos, bool fCheckEventObjects, WORD wSelfObject);

private:
    PalResources* _resources = nullptr;
    int _iThisStepFrame = 0;
};
}