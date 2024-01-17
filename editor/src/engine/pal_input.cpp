#include "pal_input.h"
#include "pal_renderer.h"
#include "global.h"
#include "imgui_impl_sdl2.h"

extern "C" {
void PAL_Shutdown(int);
}

namespace engine {

// clang-format off
static const int g_KeyMap[][2] = {
   { SDLK_UP,        PalInput::kKeyUp },
   { SDLK_KP_8,      PalInput::kKeyUp },
   { SDLK_DOWN,      PalInput::kKeyDown },
   { SDLK_KP_2,      PalInput::kKeyDown },
   { SDLK_LEFT,      PalInput::kKeyLeft },
   { SDLK_KP_4,      PalInput::kKeyLeft },
   { SDLK_RIGHT,     PalInput::kKeyRight },
   { SDLK_KP_6,      PalInput::kKeyRight },
   { SDLK_ESCAPE,    PalInput::kKeyMenu },
   { SDLK_INSERT,    PalInput::kKeyMenu },
   { SDLK_LALT,      PalInput::kKeyMenu },
   { SDLK_RALT,      PalInput::kKeyMenu },
   { SDLK_KP_0,      PalInput::kKeyMenu },
   { SDLK_RETURN,    PalInput::kKeySearch },
   { SDLK_SPACE,     PalInput::kKeySearch },
   { SDLK_KP_ENTER,  PalInput::kKeySearch },
   { SDLK_LCTRL,     PalInput::kKeySearch },
   { SDLK_PAGEUP,    PalInput::kKeyPgUp },
   { SDLK_KP_9,      PalInput::kKeyPgUp },
   { SDLK_PAGEDOWN,  PalInput::kKeyPgDn },
   { SDLK_KP_3,      PalInput::kKeyPgDn },
   { SDLK_HOME,      PalInput::kKeyHome },
   { SDLK_KP_7,      PalInput::kKeyHome },
   { SDLK_END,       PalInput::kKeyEnd },
   { SDLK_KP_1,      PalInput::kKeyEnd },
   { SDLK_r,         PalInput::kKeyRepeat },
   { SDLK_a,         PalInput::kKeyAuto },
   { SDLK_d,         PalInput::kKeyDefend },
   { SDLK_e,         PalInput::kKeyUseItem },
   { SDLK_w,         PalInput::kKeyThrowItem },
   { SDLK_q,         PalInput::kKeyFlee },
   { SDLK_f,         PalInput::kKeyForce },
   { SDLK_s,         PalInput::kKeyStatus }
};
// clang-format on
void PalInput::processEvent()
{
    SDL_Event evt;
    while (_pollEvent(&evt)) {
        // process ImGui event
        ImGui_ImplSDL2_ProcessEvent(&evt);
    }

    _updateKeyboardState();
}

int PalInput::_pollEvent(SDL_Event* event)
{
    SDL_Event evt;

    int ret = SDL_PollEvent(&evt);
    if (ret != 0 && !_input_event_filter(&evt, &_inputState)) {
        _eventFilter(&evt);
    }

    if (event != NULL) {
        *event = evt;
    }

    return ret;
}

void PalInput::_updateKeyboardState()
{
    static DWORD rgdwKeyLastTime[sizeof(g_KeyMap) / sizeof(g_KeyMap[0])] = { 0 };
    LPCBYTE keyState = (LPCBYTE)SDL_GetKeyboardState(NULL);
    int i;
    DWORD dwCurrentTime = SDL_GetTicks();

    for (i = 0; i < sizeof(g_KeyMap) / sizeof(g_KeyMap[0]); i++) {
        if (keyState[SDL_GetScancodeFromKey(g_KeyMap[i][0])]) {
            if (dwCurrentTime > rgdwKeyLastTime[i]) {
                _keyDown(g_KeyMap[i][1], (rgdwKeyLastTime[i] != 0));
                if (fEnableKeyRepeat) {
                    rgdwKeyLastTime[i] = dwCurrentTime + (rgdwKeyLastTime[i] == 0 ? 200 : 75);
                } else {
                    rgdwKeyLastTime[i] = 0xFFFFFFFF;
                }
            }
        } else {
            if (rgdwKeyLastTime[i] != 0) {
                _keyUp(g_KeyMap[i][1]);
                rgdwKeyLastTime[i] = 0;
            }
        }
    }
}

void PalInput::_keyUp(INT key)
{
    switch (key) {
    case kKeyUp:
        if (_inputState.dir == kDirNorth) {
            _inputState.dir = _inputState.prevdir;
        }
        _inputState.prevdir = kDirUnknown;
        break;

    case kKeyDown:
        if (_inputState.dir == kDirSouth) {
            _inputState.dir = _inputState.prevdir;
        }
        _inputState.prevdir = kDirUnknown;
        break;

    case kKeyLeft:
        if (_inputState.dir == kDirWest) {
            _inputState.dir = _inputState.prevdir;
        }
        _inputState.prevdir = kDirUnknown;
        break;

    case kKeyRight:
        if (_inputState.dir == kDirEast) {
            _inputState.dir = _inputState.prevdir;
        }
        _inputState.prevdir = kDirUnknown;
        break;

    default:
        break;
    }
}

void PalInput::_keyDown(INT key, BOOL fRepeat)
{
    switch (key) {
    case kKeyUp:
        if (_inputState.dir != kDirNorth && !fRepeat) {
            _inputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : _inputState.dir);
            _inputState.dir = kDirNorth;
        }
        _inputState.dwKeyPress |= kKeyUp;
        break;

    case kKeyDown:
        if (_inputState.dir != kDirSouth && !fRepeat) {
            _inputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : _inputState.dir);
            _inputState.dir = kDirSouth;
        }
        _inputState.dwKeyPress |= kKeyDown;
        break;

    case kKeyLeft:
        if (_inputState.dir != kDirWest && !fRepeat) {
            _inputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : _inputState.dir);
            _inputState.dir = kDirWest;
        }
        _inputState.dwKeyPress |= kKeyLeft;
        break;

    case kKeyRight:
        if (_inputState.dir != kDirEast && !fRepeat) {
            _inputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : _inputState.dir);
            _inputState.dir = kDirEast;
        }
        _inputState.dwKeyPress |= kKeyRight;
        break;

    default:
        _inputState.dwKeyPress |= key;
        break;
    }
}

int PalInput::_eventFilter(const SDL_Event* lpEvent)
{
    switch (lpEvent->type) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
    case SDL_WINDOWEVENT:
        if (lpEvent->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            //
            // resized the window
            //
            _renderer->resize(lpEvent->window.data1, lpEvent->window.data2);
        }
        break;

    case SDL_APP_WILLENTERBACKGROUND:
        _renderer->setPaused(true);
        break;

    case SDL_APP_DIDENTERFOREGROUND:
        _renderer->setPaused(false);
        _renderer->updateScreen(nullptr);
        break;
#else
    case SDL_VIDEORESIZE:
        //
        // resized the window
        //
        _renderer->resize(lpEvent->resize.w, lpEvent->resize.h);
        break;
#endif

    case SDL_QUIT:
        //
        // clicked on the close button of the window. Quit immediately.
        //
        PAL_Shutdown(0);
    }

    // PAL_KeyboardEventFilter(lpEvent);
    // PAL_MouseEventFilter(lpEvent);
    // PAL_JoystickEventFilter(lpEvent);
    // PAL_TouchEventFilter(lpEvent);

    //
    // All events are handled here; don't put anything to the internal queue
    //
    return 0;
}

void PalInput::init()
{
    memset((void*)&_inputState, 0, sizeof(_inputState));
    _inputState.dir = kDirUnknown;
    _inputState.prevdir = kDirUnknown;

    //
    // Check for joystick
    //
    // #if PAL_HAS_JOYSTICKS
    //     if (SDL_NumJoysticks() > 0 && g_fUseJoystick) {
    //         int i;
    //         for (i = 0; i < SDL_NumJoysticks(); i++) {
    //             if (PAL_IS_VALID_JOYSTICK(SDL_JoystickNameForIndex(i))) {
    //                 g_pJoy = SDL_JoystickOpen(i);
    //                 break;
    //             }
    //         }

    //         if (g_pJoy != NULL) {
    //             SDL_JoystickEventState(SDL_ENABLE);
    //         }
    //     }
    // #endif

    // input_init_filter();
}

void PalInput::clearKeyState()
{
    _inputState.dwKeyPress = 0;
}

}
