#include "gui_convertor.h"

namespace editor {
std::pair<const char**, size_t> obj_state_convertor()
{
    static const char* _names[] = {
        "kObjStateHidden",
        "kObjStateNormal",
        "kObjStateBlocker"
    };
    return std::make_pair(_names, sizeof(_names) / sizeof(_names[0]));
}

std::pair<const char**, size_t> obj_trigger_mode_convertor()
{
    static const char* _names[] = {
        "kTriggerNone",
        "kTriggerSearchNear",
        "kTriggerSearchNormal",
        "kTriggerSearchFar",
        "kTriggerTouchNear",
        "kTriggerTouchNormal",
        "kTriggerTouchFar",
        "kTriggerTouchFarther",
        "kTriggerTouchFarthest"
    };
    return std::make_pair(_names, sizeof(_names) / sizeof(_names[0]));
}

std::pair<const char**, size_t> obj_direction_converter()
{
    static const char* _names[] = {
        "kDirSouth",
        "kDirWest",
        "kDirNorth",
        "kDirEast",
    };
    return std::make_pair(_names, sizeof(_names) / sizeof(_names[0]));
}

}