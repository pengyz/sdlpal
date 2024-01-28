#pragma once
#include "imgui.h"
#include <cstdarg>

namespace engine {

enum class LogLevel {
    debug,
    info,
    warn,
    error,
};

class PalLog {
public:
    static inline PalLog& get()
    {
        return _log;
    }

    void clearLog();

    void addLogAp(LogLevel level, const char* fmt, va_list args);

    void addLog(LogLevel level, const char* fmt, ...) IM_FMTARGS(3);

    static const char* getLevelStr(LogLevel level);

    ImVector<char*>& getItems() { return Items; }

private:
    ImVector<char*> Items;
    static PalLog _log;
};
}