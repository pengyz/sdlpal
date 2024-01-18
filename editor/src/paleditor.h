#pragma once
#include "engine/pal_input.h"
#include "engine/pal_renderer.h"
#include "editor/native_window.h"
#include <SDL.h>

namespace engine {
class PalGlobals;
class PalResources;
}

namespace editor {

class PALEditor {
public:
    PALEditor();
    ~PALEditor();
    bool init();

    /**
     * @brief run editor loop
     *
     * @return int
     */
    int runLoop();

    /**
     * @brief deinit editor
     *
     */
    void deinit();

private:
    bool initGameEngine();

private:
    editor::NativeWindow* _mainWindow = nullptr; // main window
    engine::PalInput* _input = nullptr; // input handler
    engine::PalGlobals* _globals = nullptr; // global variables
    engine::PalResources* _resources = nullptr; // game resources
};
}
