#include "common.h"
#include "pal_editor.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../font.h"
#include "../../game.h"
#include "../../global.h"
#include "../../text.h"
#include "../../util.h"

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
    SDL_Quit();
    UTIL_Platform_Quit();
    return;
}

#ifdef __cplusplus
}
#endif

int main(int argc, char** argv)
{
    editor::PALEditor editor;
    if (!editor.init()) {
        UTIL_LogOutput(LOGLEVEL_ERROR, "editor init failed !");
        return -1;
    }
    // let's go
    editor.runLoop();
    return 0;
}