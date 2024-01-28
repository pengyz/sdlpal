#include "pal_editor.h"
#include "3rd/SDL/include/SDL.h"
#include "3rd/SDL/include/SDL_video.h"
#include "audio.h"
#include "common.h"
#include "editor/editor_meta.h"
#include "engine/pal_engine.h"
#include "engine/pal_global.h"
#include "engine/pal_resources.h"
#include "engine/pal_scene.h"
#include "engine/pal_script.h"
#include "game.h"
#include "palcfg.h"
#include "palcommon.h"
#include "script.h"
#include "util.h"
#include <imgui.h>
#include <iostream>

namespace editor {

PALEditor::PALEditor()
    : _engine(new engine::PalEngine())
{
}

PALEditor::~PALEditor()
{
    deinit();
}

bool PALEditor::init()
{
    if (SDL_Init(PAL_SDL_INIT_FLAGS) == -1) {
        TerminateOnError("Could not initialize SDL: %s.\n", SDL_GetError());
    }
    // load config
    PAL_LoadConfig(TRUE);

    UTIL_LogAddOutputCallback([](LOGLEVEL _, const char* string, const char* __) {
        const char* ptr = NULL;
        if ((ptr = strstr(string, "[SCRIPT] "))) {
            ptr += strlen("[SCRIPT] ");
        } else if ((ptr = strstr(string, "[AUTOSCRIPT] "))) {
            ptr += strlen("[AUTOSCRIPT] ") + 6;
        }
        if (ptr) {
            WORD wScriptEntry = 0;
            SCRIPTENTRY script;
            memset(&script, 0, sizeof(script));
            LPSCRIPTENTRY pScript = &script;
            auto ret = sscanf(ptr, "%4hx: %4hx %4hx %4hx %4hx\n", &wScriptEntry, &pScript->wOperation, &pScript->rgwOperand[0],
                &pScript->rgwOperand[1], &pScript->rgwOperand[2]);
            if (ret != 5) {
                printf("failed !\n");
                return;
            }
            printScript((WORD)wScriptEntry, pScript);
        } else {
            std::cout << string;
            if (string[strlen(string) - 1] != '\n')
                std::cout << std::endl;
        }
    },
        LOGLEVEL_DEBUG);

    _mainWindow = _engine->createWindow(1600, 900, "pal editor");
    if (!_engine->init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "initalize engine failed !");
        return false;
    }

    if (!editor::EditorMeta::get().load()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "load editor configs failed !");
        return false;
    }

    return true;
}

void PALEditor::deinit()
{
    if (_engine) {
        delete _engine;
        _engine = nullptr;
    }
    if (_mainWindow) {
        // do not free it, it's managed by PalEngine
        _mainWindow = nullptr;
    }
    SDL_Quit();
}

int PALEditor::runLoop()
{
    return _engine->runLoop();
}

}
