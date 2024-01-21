#pragma once
#include <cstddef>
#include <utility>

namespace editor {
std::pair<const char**, size_t> obj_state_convertor();
std::pair<const char**, size_t> obj_trigger_mode_convertor();
std::pair<const char**, size_t> obj_direction_converter();

inline std::pair<const char**, size_t> invalid_convertor()
{
    return std::make_pair(nullptr, 0);
}

}