#include "common.h"
#include "global.h"
#include "pal_loader.h"
#include "palcommon.h"
#include "play.h"
#include "script.h"
#include "util.h"
#include <ios>
#include <iostream>

int main(int argc, char** argv)
{
    PalLoader loader;
    if (!loader.load()) {
        std::cerr << "load failed !" << std::endl;
        return -1;
    }
#if 0
    std::cout << "load success..." << std::endl;
    std::cout << "level up experences: " << std::endl;
    int i = 0;
    auto g = &gpGlobals->g;
    for (int i = 1; i < sizeof(g->rgLevelUpExp) / sizeof(g->rgLevelUpExp[0]); i++) {
        std::cout << "level " << i - 1 << " -> " << i << ": " << g->rgLevelUpExp[i] << std::endl;
    }
    std::cout << "scene infos: " << std::endl;
    for (int i = 0; i < sizeof(g->rgScene) / sizeof(g->rgScene[0]); i++) {
        LPSCENE scene = &g->rgScene[i];
        // if (!scene->wMapNum && !scene->wScriptOnEnter && !scene->wScriptOnTeleport && !scene->wEventObjectIndex)
        //     break;
        std::cout << "scene " << i << ": wMapNum: " << scene->wMapNum << ", wScriptOnEnter: "
                  << scene->wScriptOnEnter << ", wScriptOnTeleport: " << scene->wScriptOnTeleport
                  << ", wEventObjectIndex: " << scene->wEventObjectIndex << std::endl;
    }
#endif

    PalScriptParser parser;
    parser.parse();

#if 0
    {
        // viewport at 0, 0
        gpGlobals->viewport = PAL_XY(0, 0);
        gpGlobals->fInMainGame = TRUE;
        gpGlobals->bCurrentSaveSlot = 0;
        // run loop
        while (true) {
            //
            // Run the main frame routine.
            //
            PAL_StartFrame();
            SDL_Delay(100);
        }
    }
#endif

    return 0;
}