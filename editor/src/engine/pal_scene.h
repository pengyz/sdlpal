#pragma once

#include "engine/pal_global.h"
#include "engine/pal_renderer.h"
#include "pal_resources.h"
#include "palcommon.h"

namespace engine {
class PalGlobals;
class PalInput;
class PalRenderer;
struct PALINPUTSTATE;

#define MAX_SPRITE_TO_DRAW 2048

typedef struct tagSPRITE_TO_DRAW {
    LPCBITMAPRLE lpSpriteFrame; // pointer to the frame bitmap
    PAL_POS pos; // position on the scene
    int iLayer; // logical layer
} SPRITE_TO_DRAW;

class PalScene {
public:
    PalScene(PalGlobals* globals, PalResources* resources, PalRenderer* renderer)
        : _globals(globals)
        , _resources(resources)
        , _renderer(renderer)
    {
    }

    void updateParty(PALINPUTSTATE* inputState);
    void updatePartyGestures(bool fWalking);
    bool checkObstacle(PAL_POS pos, bool fCheckEventObjects, WORD wSelfObject);
    void drawSprites();

private:
    void addSpriteToDraw(LPCBITMAPRLE lpSpriteFrame, int x, int y, int iLayer);
    void calcCoverTiles(SPRITE_TO_DRAW* lpSpriteToDraw);

private:
    PalGlobals* _globals = nullptr;
    PalResources* _resources = nullptr;
    PalRenderer* _renderer = nullptr;
    int _iThisStepFrame = 0;
    int _nSpriteToDraw = 0;
    SPRITE_TO_DRAW _rgSpriteToDraw[MAX_SPRITE_TO_DRAW];
};
}