#include "common.h"
#include "paleditor.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../font.h"
#include "../../game.h"
#include "../../global.h"
#include "../../res.h"
#include "../../text.h"
#include "../../util.h"
extern CONFIGURATION gConfig;

/*++
  Purpose:

    Free everything needed by the game.

  Parameters:

    exit_code -  The exit code return to OS.

  Return value:

    None.

--*/
VOID PAL_Shutdown(
    int exit_code)
{
    PAL_FreeFont();
    PAL_FreeResources();
    PAL_FreeUI();
    PAL_FreeText();
    //
    // global needs be free in last
    // since subsystems may needs config content during destroy
    // which also cleared here
    //
    PAL_FreeGlobals();
    SDL_Quit();
    UTIL_Platform_Quit();
    return;
}

#ifdef __cplusplus
}
#endif



int main(int argc, char** argv)
{
    PALEditor editor;
    if (!editor.init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "editor init failed !");
        return -1;
    }
    // let's go
    editor.runLoop();
    return 0;
}