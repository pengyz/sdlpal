#pragma once
#include "map.h"

namespace engine {

typedef enum tagLOADRESFLAG {
    kLoadGlobalData = (1 << 0), // load global data
    kLoadScene = (1 << 1), // load a scene
    kLoadPlayerSprite = (1 << 2), // load player sprites
} LOADRESFLAG,
    *LPLOADRESFLAG;

typedef struct tagRESOURCES {
    BYTE bLoadFlags;

    LPPALMAP lpMap; // current loaded map
    LPSPRITE* lppEventObjectSprites; // event object sprites
    int nEventObject; // number of event objects

    LPSPRITE rglpPlayerSprite[MAX_PLAYABLE_PLAYER_ROLES]; // player sprites
} RESOURCES, *LPRESOURCES;

class PalResources {
public:
    PalResources(class PalGlobals* globals)
        : _globals(globals)
    {
    }
    ~PalResources()
    {
        deinit();
    }
    void init();
    void deinit();
    LPPALMAP getCurrentMap();
    void freeResources();
    void loadResources();
    void setLoadFlags(BYTE bFlags);
    LPSPRITE getPlayerSprite(BYTE bPlayerIndex);
    LPCBITMAPRLE spriteGetFrame(LPCSPRITE lpSprite, INT iFrameNum);
    LPSPRITE getEventObjectSprite(WORD wEventObjectID);
    

private:
    void freePlayerSprites();
    void freeEventObjectSprites();

private:
    LPRESOURCES _resources = nullptr; // gpResources
    class PalGlobals* _globals = nullptr;
};

}