#pragma once
#include "SDL.h"
#include "palcommon.h"

namespace engine {
class PalRenderer;

struct PALINPUTSTATE {
    PALDIRECTION dir, prevdir;
    DWORD dwKeyPress;
};

class PalInput {
public:
    typedef int (*PFN_input_event_filter)(const SDL_Event*, volatile PALINPUTSTATE*);
    // clang-format off
enum PALKEY
{
   kKeyNone        = 0,
   kKeyMenu        = (1 << 0),
   kKeySearch      = (1 << 1),
   kKeyDown        = (1 << 2),
   kKeyLeft        = (1 << 3),
   kKeyUp          = (1 << 4),
   kKeyRight       = (1 << 5),
   kKeyPgUp        = (1 << 6),
   kKeyPgDn        = (1 << 7),
   kKeyRepeat      = (1 << 8),
   kKeyAuto        = (1 << 9),
   kKeyDefend      = (1 << 10),
   kKeyUseItem     = (1 << 11),
   kKeyThrowItem   = (1 << 12),
   kKeyFlee        = (1 << 13),
   kKeyStatus      = (1 << 14),
   kKeyForce       = (1 << 15),
   kKeyHome        = (1 << 16),
   kKeyEnd         = (1 << 17),
};
    // clang-format on

    PalInput(engine::PalRenderer* renderer)
        : _renderer(renderer)
    {
    }

    void init();

    void processEvent();

    void clearKeyState();

    PALINPUTSTATE* getInputState() { return &_inputState; }

private:
    int _pollEvent(SDL_Event* event);
    void _updateKeyboardState();
    void _keyUp(INT key);
    void _keyDown(INT key, BOOL fRepeat);
    int _eventFilter(const SDL_Event* lpEvent);

private:
    PALINPUTSTATE _inputState;
    PFN_input_event_filter _input_event_filter = [](const SDL_Event* event, volatile PALINPUTSTATE* state) { return 0; };
    engine::PalRenderer* _renderer = nullptr;
    bool fEnableKeyRepeat = false;
};
}
